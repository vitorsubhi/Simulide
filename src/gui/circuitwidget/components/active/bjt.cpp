/***************************************************************************
 *   Copyright (C) 2018 by santiago González                               *
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

#include <math.h>

#include "bjt.h"
#include "itemlibrary.h"
#include "circuitwidget.h"
#include "connector.h"
#include "simulator.h"
#include "circuit.h"
#include "pin.h"

#include "doubleprop.h"
#include "boolprop.h"

Component* BJT::construct( QObject* parent, QString type, QString id )
{ return new BJT( parent, type, id ); }

LibraryItem* BJT::libraryItem()
{
    return new LibraryItem(
        tr( "BJT" ),
        tr( "Active" ),
        "bjt.png",
        "BJT",
        BJT::construct );
}

BJT::BJT( QObject* parent, QString type, QString id )
   : Component( parent, type, id )
   , eBJT( id )
{
    m_area =  QRectF( -12, -14, 28, 28 );
    setLabelPos(18, 0, 0);

    m_pin.resize( 3 );
    m_ePin[0] = m_pin[0] = new Pin( 90,  QPoint( 8,-16), id+"-collector", 0, this );
    m_ePin[1] = m_pin[1] = new Pin( 270, QPoint( 8, 16), id+"-emiter"   , 1, this );
    m_ePin[2] = m_pin[2] = new Pin( 180, QPoint(-16, 0), id+"-base"     , 0, this );

    Simulator::self()->addToUpdateList( this );
    initialize();

    addPropGroup( { tr("Main"), {
new BoolProp<BJT>( "PNP"  , tr("PNP")      ,"" , this, &BJT::pnp      , &BJT::setPnp ),
new DoubProp<BJT>( "Gain" , tr("Gain")     ,"" , this, &BJT::gain     , &BJT::setGain ),
new DoubProp<BJT>( "Vcrit", tr("Threshold"),"V", this, &BJT::threshold, &BJT::setThreshold ),
    }} );
}
BJT::~BJT(){}

void BJT::updateStep()
{
    if( Circuit::self()->animate() ) update();
}

void BJT::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );
    
    if( Circuit::self()->animate() && fabs(m_baseCurr) > 1e-4 ) p->setBrush( Qt::yellow );
    else                                                        p->setBrush( Qt::white );

    p->drawEllipse( m_area );
    p->drawLine( -12, 0,-4, 0 );
    p->drawLine(  -4,-8,-4, 8 );
    p->drawLine( -4,-4, 8,-12 );
    p->drawLine( -4, 4, 8, 12 );
    
    p->setBrush( Qt::black );
    if( m_PNP ){
        QPointF points[3] = {
        QPointF( 0.1, 6.8 ),
        QPointF( 2.4, 10 ),
        QPointF( 4, 7.5 ) };
        p->drawPolygon(points, 3);
    }else{
        QPointF points[3] = {
        QPointF( 6, 10.7 ),
        QPointF( 2.4, 10 ),
        QPointF( 4, 7.5 ) };
        p->drawPolygon(points, 3);
}   }
