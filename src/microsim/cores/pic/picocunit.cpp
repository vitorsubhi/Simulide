﻿/***************************************************************************
 *   Copyright (C) 2021 by santiago González                               *
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

#include "picocunit.h"
#include "datautils.h"
#include "mcupin.h"
#include "e_mcu.h"
#include "simulator.h"

PicPwmUnit* PicOcUnit::createPwmUnit( eMcu* mcu, QString name, int type ) // Static
{
    if( type == 00 ) return new PicPwmUnit00( mcu, name );
    if( type == 01 ) return new PicPwmUnit01( mcu, name );
    return NULL;
}

PicOcUnit::PicOcUnit( eMcu* mcu, QString name )
         : McuOcUnit( mcu, name )
{
    m_enhanced = name.contains("+");
    m_resetTimer = false;

    m_GODO = getRegBits( "GO/DONE", mcu );
}
PicOcUnit::~PicOcUnit( ){}

void PicOcUnit::runEvent()  // Compare match
{
    if( !m_enabled ) return;

    if( m_resetTimer ) // 1 Timer cycle after last match
    {
        m_resetTimer = false;
        m_timer->resetTimer();
        return;
    }
    if( m_specEvent )
    {
         m_resetTimer = true;
         uint64_t cycles = m_timer->scale()*m_mcu->psCycle();
         Simulator::self()->addEvent( cycles, this ); // Reset Timer next Timer cycle

         m_mcu->writeReg( m_GODO.regAddr, *m_GODO.reg | m_GODO.mask );  // Set ADC GO/DONE bit
    }
    m_interrupt->raise();   // Trigger interrupt
    drivePin( m_comAct );
}

void PicOcUnit::configure( uint8_t CCPxM )  // CCPxM0,CCPxM1,CCPxM2,CCPxM3
{
    m_specEvent = false;

    if( m_enhanced && CCPxM == 2 ) setOcActs( ocTOG, ocNON ); // Toggle OC Pin
    else{
        switch( CCPxM ) {
            case 8: setOcActs( ocSET, ocCLR ); break; // Set OC Pin
            case 9: setOcActs( ocCLR, ocSET ); break; // Clear OC Pin
            //case 10:                                  // Only interrupt
            case 11: m_specEvent = true;                // Special event
        }
    }
    m_ocPin->controlPin( true, false ); // Connect/Disconnect PORT
    m_ocPin->setOutState( false );
    m_enabled = true;
}

//------------------------------------------------------
//-- PIC PWM Unit --------------------------------------

PicPwmUnit::PicPwmUnit( eMcu* mcu, QString name )
          : McuOcUnit( mcu, name )
{
    m_enhanced = name.contains("+");
}
PicPwmUnit::~PicPwmUnit( ){}

void PicPwmUnit::runEvent()  // Compare match
{
    if( !m_enabled ) return;

    m_interrupt->raise();   // Trigger interrupt
    drivePin( m_comAct );
}

void PicPwmUnit::configure( uint8_t newCCPxCON )
{
    m_cLow = getRegBitsVal( newCCPxCON, m_DCxB );

    setOcActs( ocCLR, ocSET ); // Clear Out on match, set on TOV

    if( m_enhanced ) // PWM Mode Enhanced
    {
        switch( newCCPxCON & 0x0F ) {
            case 12: // All Pins Active High
            {

            } break;
            case 13: // P1A,C Active High; P1B,D Active Low;
            {

            } break;
            case 14: // P1A,C Active Low; P1B,D Active High;
            {

            } break;
            case 15: // All Pins Active Low
            {

            } break;
    }   }
    m_ocPin->controlPin( true, false ); // Connect/Disconnect PORT
    m_ocPin->setOutState( false );
    m_enabled = true;
}

void  PicPwmUnit::sheduleEvents( uint32_t ovf, uint32_t countVal, int ) // Use CCPRxL + 2 bits from CCPxCON
{
    m_comMatch = (m_CCPRxL<<2) | m_cLow;
    McuOcUnit::sheduleEvents( ovf, countVal, 2 );
}

void PicPwmUnit::ocrWriteL( uint8_t val ) // CCPRxL
{
    m_CCPRxL = val;
    m_comMatch = (m_comMatch & 0xFF00) | val;
    //m_timer->updtCount();
    sheduleEvents( m_timer->ovfMatch(), m_timer->getCount() );
}

//------------------------------------------------------
//-- PIC 16f88x PWM Unit -------------------------------

PicPwmUnit00::PicPwmUnit00( eMcu* mcu, QString name )
            : PicPwmUnit( mcu, name )
{
    QString n = name.right(1); // name="PWM+2" => n="2"

    m_DCxB  = getRegBits( "DC"+n+"B0,DC"+n+"B1", mcu );
    if( m_enhanced ) m_PxM = getRegBits( "P"+n+"M0,P"+n+"M1", mcu );
}
PicPwmUnit00::~PicPwmUnit00( ){}

//------------------------------------------------------
//-- PIC 16f627 PWM Unit -------------------------------

PicPwmUnit01::PicPwmUnit01( eMcu* mcu, QString name )
            : PicPwmUnit( mcu, name )
{
    QString n = name.right(1); // name="PWM+2" => n="2"

    m_DCxB  = getRegBits( "CCP"+n+"Y,CCP"+n+"X", mcu );
    //if( m_enhanced ) m_PxM = getRegBits( "P"+n+"M0,P"+n+"M1", mcu );
}
PicPwmUnit01::~PicPwmUnit01( ){}

