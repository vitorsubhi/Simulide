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

#ifndef LEDSMD_H
#define LEDSMD_H

#include "ledbase.h"

class MAINMODULE_EXPORT LedSmd : public LedBase
{
    public:
        LedSmd( QObject* parent, QString type, QString id, QRectF area, ePin* pin0=NULL, ePin* pin1=NULL );
        ~LedSmd();

    protected:
        void drawBackground( QPainter* p );
        void drawForeground( QPainter* p );
};

#endif
