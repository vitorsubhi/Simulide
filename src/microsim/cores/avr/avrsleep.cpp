/***************************************************************************
 *   Copyright (C) 2022 by santiago González                               *
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

//#include <QDebug>

#include "avrsleep.h"
#include "e_mcu.h"
#include "mcuwdt.h"
#include "datautils.h"

AvrSleep::AvrSleep( eMcu* mcu, QString name )
        : McuSleep( mcu, name )
{
    m_SM = getRegBits( "SM0,SM1,SM2", mcu );
    m_SE = getRegBits( "SE", mcu );

    //Interrupt* inte = NULL;

    //m_wakeUps.emplace_back( m_mcu->watchDog()->getInterrupt() );
}
AvrSleep::~AvrSleep(){}

void AvrSleep::initialize()
{
    m_sleepMode = sleepNone;
}

void AvrSleep::configureA( uint8_t newVal )
{
    bool enabled = getRegBitsBool( newVal, m_SE );
    if( m_enabled != enabled )
    {
        m_enabled = enabled;
        if( !enabled )
        {
            for( Interrupt* inte : m_wakeUps )
            {
                if( inte ) inte->callBack( this, false);
            }
        }
    }
    sleepMode_t mode = (sleepMode_t)getRegBitsVal( newVal, m_SM );
    if( m_sleepMode != mode )
    {
        m_sleepMode = mode;
        switch( mode ) {
            case sleepIdle: break;
            case sleepAdcNR: break;
            case sleepPowDo: break;
            case sleepPowSa: break;
            case sleepStand: break;
            case sleepExtSt: break;
        default:
            break;
        }
    }
}


/*
//------------------------------------------------------
//-- AVR ADC Type 0 ------------------------------------

AvrSleep00::AvrSleep00( eMcu* mcu, QString name )
          : AvrSleep( mcu, name )
{
    m_SM = getRegBits( "SM0,SM1,SM2", mcu );
}
AvrSleep::~AvrSleep(){}

//------------------------------------------------------
//-- AVR ADC Type 1 ------------------------------------

AvrSleep00::AvrSleep00( eMcu* mcu, QString name )
          : AvrSleep( mcu, name )
{
    m_SM = getRegBits( "SM0,SM1", mcu );
}
AvrSleep::~AvrSleep(){}
*/
