/***************************************************************************
 *   Copyright (C) 2017 by santiago González                               *
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
 
#include <QDir>
#include <QDebug>
#include <QSettings>
#include <QCloseEvent>

#include "compplugindialog.h"
#include "componentselector.h"
#include "mainwindow.h"
#include "ui_compplugin.h"

CompPluginDialog::CompPluginDialog( QWidget *parent )
                : QDialog( parent )
                , ui( new Ui::ComponentPlugins )
{
    ui->setupUi(this);

    m_compList = ui->compList;
    m_initialized = false;

    connect( m_compList, SIGNAL( itemChanged( QListWidgetItem* )),
                   this, SLOT( slotItemChanged( QListWidgetItem* )), Qt::UniqueConnection );
}

CompPluginDialog::~CompPluginDialog() { delete ui; }

void CompPluginDialog::addItem( QTreeWidgetItem* item )
{
    QString itemName = item->text( 0 );

    QListWidgetItem* listItem = new QListWidgetItem( itemName );
    listItem->setCheckState( item->isHidden() ? Qt::Unchecked : Qt::Checked );

    m_compList->addItem( listItem );
    m_qtwItem[ listItem ] = item;

    int childCount = item->childCount();
    if( childCount > 0 )
    {
        listItem->setBackgroundColor( QColor(240, 235, 245));
        listItem->setForeground( QBrush( QColor( 110, 95, 50 )));

        for( int i=0; i<childCount; i++ ) addItem( item->child( i ) );
    }
    else listItem->setIcon( QIcon(":/blanc.png") );
}

void CompPluginDialog::setPluginList()
{
    if( m_initialized ) return;

    QList<QTreeWidgetItem*> itemList = ComponentSelector::self()->findItems("",Qt::MatchStartsWith);

    for( QTreeWidgetItem* item : itemList ) addItem( item );

    m_initialized = true;
}

void CompPluginDialog::slotItemChanged( QListWidgetItem* item )
{
    if( !m_initialized ) return;
    setItemVisible( item, item->checkState() );
}

void CompPluginDialog::setItemVisible( QListWidgetItem* listIt, bool visible )
{
    QTreeWidgetItem* treeItem = m_qtwItem[ listIt ];
    treeItem->setHidden( !visible );

    int childCount = treeItem->childCount();
    if( childCount > 0 )
    {
        for( int i=0; i<childCount; i++ )
        {
            QListWidgetItem* listItem = m_qtwItem.keys( treeItem->child( i ) ).at(0);

            if( visible ) listItem->setCheckState( Qt::Checked );
            else          listItem->setCheckState( Qt::Unchecked );
}   }   }

void CompPluginDialog::reject()
{
    this->setVisible( false );

    for( QListWidgetItem* listItem : m_compList->findItems("",Qt::MatchStartsWith) )
    {
        bool hidden = ( listItem->checkState() == Qt::Unchecked );
        MainWindow::self()->settings()->setValue( listItem->text()+"/hidden", hidden );
}   }

#include "moc_compplugindialog.cpp"
