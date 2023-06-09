/***************************************************************************
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

#ifndef AVRADC_H
#define AVRADC_H

#include "mcuadc.h"
#include "mcutypes.h"

class eMcu;
class IoPin;
class AvrTimer800;
class AvrTimer16bit;
class McuOcUnit;

class MAINMODULE_EXPORT AvrAdc : public McuAdc
{
    public:
        AvrAdc( eMcu* mcu, QString name );
        ~AvrAdc();

 static AvrAdc* createAdc( eMcu* mcu, QString name, int type );

        virtual void initialize() override;

        virtual void configureA( uint8_t newADCSRA ) override;
        virtual void configureB( uint8_t newADCSRB ) override;
        virtual void setChannel( uint8_t newADMUX ) override;
        virtual void callBack() override { if( !m_converting ) startConversion(); }

    protected:
        virtual void autotriggerConf(){;}
        virtual void endConversion() override;

        bool m_autoTrigger;
        bool m_freeRunning;
        uint m_initCycles;

        uint8_t m_refSelect;
        uint8_t m_trigger;

        //uint8_t* m_ADCSRA;
        regBits_t m_ADEN;
        regBits_t m_ADSC;
        regBits_t m_ADATE;
        regBits_t m_ADIF;
        regBits_t m_ADPS;

        // ADCSB
        regBits_t m_ADTS;

        //ADMUX
        regBits_t m_MUX;
        regBits_t m_ADLAR;
        regBits_t m_REFS;

        IoPin* m_aRefPin;
        IoPin* m_aVccPin;

        AvrTimer800*     m_timer0;
        AvrTimer16bit*  m_timer1;

        McuOcUnit* m_t0OCA;
        McuOcUnit* m_txOCB;
};

class MAINMODULE_EXPORT AvrAdc00 : public AvrAdc
{
    public:
        AvrAdc00( eMcu* mcu, QString name );
        ~AvrAdc00();

    protected:
        virtual void autotriggerConf() override;
        virtual void updtVref() override;
};

class MAINMODULE_EXPORT AvrAdc01 : public AvrAdc00
{
    public:
        AvrAdc01( eMcu* mcu, QString name );
        ~AvrAdc01();

    protected:
        virtual void autotriggerConf() override;
};

class MAINMODULE_EXPORT AvrAdc02 : public AvrAdc00
{
    public:
        AvrAdc02( eMcu* mcu, QString name );
        ~AvrAdc02();

    protected:
        virtual void updtVref() override;
};

class MAINMODULE_EXPORT AvrAdc03 : public AvrAdc00
{
    public:
        AvrAdc03( eMcu* mcu, QString name );
        ~AvrAdc03();

        virtual void startConversion() override;

    protected:
        virtual void specialConv() override;
};

class MAINMODULE_EXPORT AvrAdc04 : public AvrAdc03
{
    public:
        AvrAdc04( eMcu* mcu, QString name );
        ~AvrAdc04();

        virtual void configureB( uint8_t newADCSRB ) override;

    protected:

        // ADCSB
        regBits_t m_MUX5;
};

class MAINMODULE_EXPORT AvrAdc10 : public AvrAdc
{
    public:
        AvrAdc10( eMcu* mcu, QString name );
        ~AvrAdc10();

    protected:
        virtual void autotriggerConf() override;
        virtual void updtVref() override;
};

class MAINMODULE_EXPORT AvrAdc11 : public AvrAdc10
{
    public:
        AvrAdc11( eMcu* mcu, QString name );
        ~AvrAdc11();

    protected:
        virtual void updtVref() override;
};
#endif
