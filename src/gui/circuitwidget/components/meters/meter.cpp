/***************************************************************************
 *   Copyright (C) 2012 by santiago González                               *
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

#include <QtMath>
#include <QFont>
#include <QPainter>

#include "meter.h"
#include "simulator.h"
#include "iopin.h"
#include "label.h"
#include "utils.h"

#include "boolprop.h"

Meter::Meter( QObject* parent, QString type, QString id )
     : Component( parent, type, id )
     , eResistor( id )
     , m_display( this )
{
    m_area = QRectF( -24, -24, 50, 32 );
    m_graphical = true;
    m_switchPins = false;

    m_pin.resize( 3 );
    m_ePin[0] = m_pin[0] = new Pin( 270, QPoint(-8, 16), id+"-lPin", 0, this);
    m_pin[0]->setColor( Qt::red );

    m_ePin[1] = m_pin[1] = new Pin( 270, QPoint(8, 16), id+"-rPin", 1, this);

    m_pin[2] = m_outPin = new IoPin( 0, QPoint(32,-8), id+"-outnod", 0, this, source );
    m_outPin->setOutHighV( 0 );
    m_outPin->setOutState( true );

    m_idLabel->setPos(-12,-24);
    setLabelPos(-24,-40, 0);

    QFont f( "Helvetica [Cronyx]", 10, QFont::Bold );
    f.setPixelSize(12);
    m_display.setFont(f);
    m_display.setBrush(  Qt::yellow );
    m_display.setPos( -22, -22 );
    m_display.setVisible( true );

    Simulator::self()->addToUpdateList( this );

    /*addPropGroup( { tr("Main"), {
new BoolProp<Meter>( "SwitchPins", tr("Switch Pins"),"", this, &Meter::swithchPins, &Meter::setSwitchPins )
    } } );*/
}
Meter::~Meter(){}

bool Meter::setPropStr( QString prop, QString val )
{
    if( prop =="SwitchPins" )       // Old: TODELETE
    {
        if( val == "true" ) { m_Hflip = -1; setflip(); }
    }
    else return Component::setPropStr( prop, val );
    return true;
}

void Meter::updateStep()
{
    QString sign = " ";
    QString mult = " ";
    int decimals = 3;
    double value = qFabs(m_dispValue);

    if( value < 1e-9 ) value = 0;
    else{
        value *= 1e12;
        if( m_dispValue < 0 ) sign = "-";
        valToUnit( value, mult, decimals )
    }
    if( value > 999 )
    {
        m_display.setText( " ----" );
        m_crashed = true;
    }
    else m_display.setText( sign+QString::number( value,'f', decimals ).left(5)+"\n"+mult+m_unit );

    m_outPin->setOutHighV( m_dispValue );
    m_outPin->setOutState( true );
}

void Meter::setSwitchPins( bool s )
{
    if( s == m_switchPins ) return;
    m_switchPins = s;

    qreal x0 = s ? 8 :-8;
    qreal x1 = s ?-8 : 8;

    m_pin[0]->setX( x0 );
    m_pin[1]->setX( x1 );

    m_pin[0]->isMoved();
    m_pin[1]->isMoved();
}

void Meter::setflip()
{
    Component::setflip();

    int xlabelpos = -22*m_Hflip;
    int ylabelpos = -22*m_Vflip;
    if( m_Vflip < 0 ) ylabelpos -= 16;

    m_display.setPos( xlabelpos, ylabelpos );
    m_display.setTransform( QTransform::fromScale( m_Hflip, m_Vflip ) );
}

void Meter::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );
    p->setBrush( Qt::black);
    p->drawRect( QRectF( -24, -24, 48, 32 ) );

    /*if( m_hidden ) return;

    QPointF points[3] = {
    QPointF( 26,-11 ),
    QPointF( 31, -8 ),
    QPointF( 26, -5 )   };
    p->drawPolygon(points, 3);*/
}

#include "moc_meter.cpp"
