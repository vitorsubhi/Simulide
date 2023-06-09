/***************************************************************************
 *   Copyright (C) 2016 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/

#include "aip31068_i2c.h"
#include "itemlibrary.h"
#include "simulator.h"
#include "iopin.h"
#include "utils.h"

#include "doubleprop.h"
#include "intprop.h"

Component* Aip31068_i2c::construct( QObject* parent, QString type, QString id )
{ return new Aip31068_i2c( parent, type, id ); }

LibraryItem* Aip31068_i2c::libraryItem()
{
    return new LibraryItem(
        tr( "Aip31068_i2c" ),
        tr( "Displays" ),
        "aip31068.png",
        "Aip31068_i2c",
        Aip31068_i2c::construct );
}

Aip31068_i2c::Aip31068_i2c( QObject* parent, QString type, QString id )
            : Hd44780_Base( parent, type, id )
            , TwiModule( id )
{
    m_address = m_cCode = 0x3E;

    m_pin.resize( 2 );

    m_pinSda = new IoPin( 270, QPoint(16, 8), id+"PinSDA", 0, this, openCo );
    m_pinSda->setLabelText( "SDA" );
    m_pin[0] = m_pinSda;
    TwiModule::setSdaPin( m_pinSda );

    m_clkPin = new IoPin( 270, QPoint(24, 8), id+"PinSCL", 0, this, openCo );
    m_clkPin->setLabelText( "SCL" );
    m_pin[1] = m_clkPin;
    TwiModule::setSclPin( m_clkPin );

    Simulator::self()->addToUpdateList( this );
    
    initialize();

    addPropGroup( { tr("Main"), {
new IntProp <Aip31068_i2c>( "Rows"        ,tr("Rows")         ,"_Rows"   ,this,&Aip31068_i2c::rows,   &Aip31068_i2c::setRows  ,"uint" ),
new IntProp <Aip31068_i2c>( "Cols"        ,tr("Columns")      ,"_Columns",this,&Aip31068_i2c::cols,   &Aip31068_i2c::setCols,"uint"  ),
new IntProp <Aip31068_i2c>( "Control_Code",tr("I2C Address")  ,""        ,this,&Aip31068_i2c::cCode,  &Aip31068_i2c::setCcode,"uint" ),
new DoubProp<Aip31068_i2c>( "Frequency"   ,tr("I2C Frequency"),"_KHz"    ,this,&Aip31068_i2c::freqKHz,&Aip31068_i2c::setFreqKHz ),
    }} );
}
Aip31068_i2c::~Aip31068_i2c(){}

void Aip31068_i2c::updateStep() { update(); }

void Aip31068_i2c::initialize()
{
    TwiModule::initialize();

    m_controlByte = 0;
    m_phase = 3;

    Hd44780_Base::init();
}

void Aip31068_i2c::stamp()
{
    setMode( TWI_SLAVE );
}

void Aip31068_i2c::voltChanged()             // Called when clock Pin changes
{
    TwiModule::voltChanged();
    if( m_i2cState == I2C_STOP ) m_phase = 3;
}

void Aip31068_i2c::startWrite() { m_phase = 0; }

void Aip31068_i2c::readByte()
{
    if( m_phase == 0 )
    {
        m_phase++;
        m_controlByte = m_rxReg;
    }
    else if( m_phase == 1 )
    {
        m_phase++;
        int data = TwiModule::byteReceived();
        int rs = m_controlByte & 0x40;

        if( rs ) writeData(data);
        else     proccessCommand(data);
    }
    TwiModule::readByte();
}

void Aip31068_i2c::showPins( bool show )
{
    m_pinSda->setVisible( show );
    m_clkPin->setVisible( show );
}
