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

#ifndef ITEMLIBRARY_H
#define ITEMLIBRARY_H

#include <QCoreApplication>

#include "component.h"

class LibraryItem;

class MAINMODULE_EXPORT ItemLibrary
{
    Q_DECLARE_TR_FUNCTIONS( ItemLibrary )
    
    public:
        ItemLibrary();
        ~ItemLibrary();

 static ItemLibrary* self() { return m_pSelf; }

        const QList<LibraryItem*> items() const;

        LibraryItem*  libraryItem( const QString type ) const;

        LibraryItem*  itemByName( const QString name ) const;

        void addItem( LibraryItem* item );
        
        void loadItems();
    
    protected:
 static ItemLibrary* m_pSelf;

        QList<LibraryItem*> m_items;
        
        friend ItemLibrary*  itemLibrary();
};


class MAINMODULE_EXPORT LibraryItem
{
    public:
        LibraryItem( const QString &name, const QString &category, const QString &iconName,
                     const QString type, createItemPtr createItem );
        
        ~LibraryItem();

        QString name()     const { return m_name; }
        QString category() const { return m_category; }
        QString iconfile() const { return m_iconfile; }
        QString type()     const { return m_type; }

        createItemPtr createItemFnPtr() const { return createItem; }

    private:
        QString m_name;
        QString m_category;
        QString m_iconfile;
        QString m_type;

        createItemPtr createItem;
};

#endif
