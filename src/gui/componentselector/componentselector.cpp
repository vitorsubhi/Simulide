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

#include <QApplication>
#include <QDomDocument>
#include <QMessageBox>
#include <QXmlStreamReader>
#include <QMimeData>
#include <QSettings>
#include <QDebug>
#include <QDrag>
#include <QMenu>

#include "componentselector.h"
#include "mainwindow.h"

ComponentSelector* ComponentSelector::m_pSelf = NULL;

ComponentSelector::ComponentSelector( QWidget* parent )
                 : QTreeWidget( parent )
                 , m_pluginsdDialog( this )
                 , m_itemLibrary()
{
    m_pSelf = this;

    m_pluginsdDialog.setVisible( false );

    setDragEnabled(true);
    setDragDropMode( QAbstractItemView::DragOnly );
    //setAlternatingRowColors(true);
    setIndentation( 12 );
    setRootIsDecorated( true );
    setCursor( Qt::OpenHandCursor );
    headerItem()->setHidden( true );
    setIconSize( QSize( 36, 24 ));

    LoadLibraryItems();

    setContextMenuPolicy( Qt::CustomContextMenu );

    connect( this, SIGNAL(customContextMenuRequested(const QPoint&)),
             this, SLOT  (slotContextMenu(const QPoint&)), Qt::UniqueConnection );

    connect( this, SIGNAL(itemPressed(QTreeWidgetItem*, int)),
             this, SLOT  (slotItemClicked(QTreeWidgetItem*, int)), Qt::UniqueConnection );
}
ComponentSelector::~ComponentSelector(){}

void ComponentSelector::LoadLibraryItems()
{
    for( LibraryItem* libItem : m_itemLibrary.items() ) addItem( libItem );
}

void ComponentSelector::LoadCompSetAt( QDir compSetDir )
{
    compSetDir.setNameFilters( QStringList( "*.xml" ) );

    QStringList xmlList = compSetDir.entryList( QDir::Files );

    if( xmlList.isEmpty() ) return;                  // No comp sets to load

    qDebug() << "\n" << tr("    Loading Component sets at:")<< "\n" << compSetDir.absolutePath()<<"\n";

    for( QString compSetName : xmlList )
    {
        QString compSetFilePath = compSetDir.absoluteFilePath( compSetName );
        if( !compSetFilePath.isEmpty( ))  loadXml( compSetFilePath );
    }
    qDebug() << "\n";
}

void ComponentSelector::loadXml( const QString &setFile )
{
    QFile file( setFile );
    if( !file.open(QFile::ReadOnly | QFile::Text) )
    {
          QMessageBox::warning(0, "ComponentSelector::loadXml", tr("Cannot read file %1:\n%2.").arg(setFile).arg(file.errorString()));
          return;
    }
    QXmlStreamReader reader(&file);
    if( reader.readNextStartElement() )
    {
        if( reader.name() != "itemlib" )
        {
            QMessageBox::warning(0, "ComponentSelector::loadXml"
                                 , tr("Error parsing file (itemlib):\n%1.").arg(setFile) );
            file.close();
            return;
        }
        while( reader.readNextStartElement() )
        {
            if( reader.name() != "itemset" ) { reader.skipCurrentElement(); continue;}

            QString category = reader.attributes().value("category").toString();

            QStringList catPath = category.split( "/" );
            category = "";
            for( QString cat : catPath )
            {
                cat = QObject::tr( cat.toLocal8Bit() );
                category.append( "/"+cat );
            }
            category.remove( 0, 1 ); // convert "/rootCat/category" to "rootCat/category"

            QString icon = "";
            if( reader.attributes().hasAttribute("icon") )
            {
                icon = reader.attributes().value("icon").toString();
                if( !icon.startsWith(":/") )
                    icon.prepend( MainWindow::self()->getFilePath("data/images") + "/");
            }
            QTreeWidgetItem* catItem = getCategory( category, icon );

            QString type = reader.attributes().value("type").toString();

            while( reader.readNextStartElement() )
            {
                if( reader.name() == "item")
                {
                    icon = "";
                    if( reader.attributes().hasAttribute("icon") )
                    {
                        icon = reader.attributes().value("icon").toString();
                        if( !icon.startsWith(":/") )
                            icon.prepend( MainWindow::self()->getFilePath("data/images") + "/");
                    }
                    QString name = reader.attributes().value("name").toString();

                    m_xmlFileList[ name ] = setFile;   // Save xml File used to create this item
                    if( reader.attributes().hasAttribute("info") )
                        name += "???"+reader.attributes().value("info").toString();

                    addItem( name, catItem, icon, type );
                    reader.skipCurrentElement();
    }   }   }   }
    QString compSetName = setFile.split( "/").last();
    m_compSetUnique.append( compSetName );
    qDebug() << tr("        Loaded Component set:           ") << compSetName;
}

void ComponentSelector::addItem( LibraryItem* libItem )
{
    QString category = libItem->category();
    if( category != "")
    {
        QString icon = ":/"+libItem->iconfile();
        QTreeWidgetItem* catItem = getCategory( category );
        addItem( libItem->name(), catItem, icon, libItem->type() );
}   }

void ComponentSelector::addItem( QString caption, QTreeWidgetItem* catItem, QString icon, QString type )
{
    QStringList nameFull = caption.split( "???" );
    QString         name = nameFull.first();
    QString info = "";
    if( nameFull.size() > 1 ) info = "   "+nameFull.last();
    if( !m_categories.contains( name, Qt::CaseSensitive ) )
        m_categories.append( name );

    bool hidden = MainWindow::self()->settings()->value( name+"/hidden" ).toBool();

    QTreeWidgetItem* item = new QTreeWidgetItem(0);
    QFont font;
    font.setFamily("Ubuntu");
    font.setBold( true );
    
    double fontScale = MainWindow::self()->fontScale();
    if( type == "" ) font.setPixelSize( 12*fontScale );
    else             font.setPixelSize( 11*fontScale );
    
    item->setFlags( QFlag(32) );
    item->setFont( 0, font );
    item->setIcon( 0, QIcon( QPixmap( icon ) ) );
    item->setText( 0, name+info );
    item->setData( 0, Qt::UserRole, type );

    if( ( type == "Subcircuit" )||( type == "MCU" ) )
         item->setData( 0, Qt::WhatsThisRole, name );
    else item->setData( 0, Qt::WhatsThisRole, type );
    
    catItem->addChild( item );
    item->setHidden( hidden );
    if( MainWindow::self()->settings()->contains( name+"/collapsed" ) )
    {
        bool expanded = !MainWindow::self()->settings()->value( name+"/collapsed" ).toBool();
        item->setExpanded( expanded );
}   }

QTreeWidgetItem* ComponentSelector::getCategory( QString _category, QString icon )
{
    QTreeWidgetItem* catItem = NULL;

    QStringList catPath = _category.split( "/" );
    bool      isRootCat = (catPath.size() == 1);
    QString    category = catPath.takeLast();

    if( m_categories.contains( category, Qt::CaseSensitive ) )    // Find Category
    {
        QList<QTreeWidgetItem*> list = findItems( category, Qt::MatchExactly | Qt::MatchRecursive );
        if( !list.isEmpty() ) catItem = list.first();
    }
    else                                            // Create new Category
    {
        bool c_hidden = false;
        bool expanded = false;

        if( isRootCat )                              // Is Main Category
        {
            catItem = new QTreeWidgetItem( this );
            catItem->setTextColor( 0, QColor( 110, 95, 50 )/*QColor(255, 230, 200)*/ );
            catItem->setBackground( 0, QBrush(QColor(240, 235, 245)) );
            expanded = true;
        }
        else catItem = new QTreeWidgetItem(0);

        catItem->setIcon( 0, QIcon( QPixmap( icon ) ) );
        catItem->setFlags( QFlag(32) );
        QFont font = catItem->font(0);
        font.setPixelSize( 13*MainWindow::self()->fontScale() );
        font.setWeight(75);
        catItem->setFont( 0, font );
        catItem->setText( 0, category );
        catItem->setChildIndicatorPolicy( QTreeWidgetItem::ShowIndicator );
        m_categories.append( category );

        if( !catPath.isEmpty() )
        {
            QString topCat = catPath.takeLast();

            QList<QTreeWidgetItem*> list = findItems( topCat, Qt::MatchExactly | Qt::MatchRecursive );
            if( list.isEmpty() ) catPath.clear(); // Error: root category doesn't exist, force addTopLevelItem( catItem )
            else{
                QTreeWidgetItem* topItem = list.first();
                topItem->addChild( catItem );
            }
        }
        if( catPath.isEmpty() ) addTopLevelItem( catItem ); // Is root category or root category doesn't exist

        if( MainWindow::self()->settings()->contains( category+"/hidden" ) )
            c_hidden =  MainWindow::self()->settings()->value( category+"/hidden" ).toBool();

        if( MainWindow::self()->settings()->contains( category+"/collapsed" ) )
            expanded = !MainWindow::self()->settings()->value( category+"/collapsed" ).toBool();

        catItem->setExpanded( expanded );
        catItem->setHidden( c_hidden );
    }
    return catItem;
}

void ComponentSelector::slotItemClicked( QTreeWidgetItem* item, int  )
{
    if( !item ) return;
    
    QString type = item->data( 0, Qt::UserRole ).toString();
    if( type == "" ) return;

    QString name = item->data (0, Qt::WhatsThisRole ).toString(); //item->text(0);
    QMimeData* mimeData = new QMimeData;
    mimeData->setText( name );
    mimeData->setHtml( type );              // esto hay que revisarlo
    
    QDrag* drag = new QDrag( this );
    drag->setMimeData( mimeData );
    drag->exec( Qt::CopyAction | Qt::MoveAction, Qt::CopyAction );
}

void ComponentSelector::slotContextMenu( const QPoint& point )
{
    QMenu menu;
    
    QAction* managePluginAction = menu.addAction( QIcon(":/fileopen.png"),tr("Manage Components") );
    connect( managePluginAction, SIGNAL(triggered()), this, SLOT(slotManageComponents()) );

    //menu->addSeparator();
    
    menu.exec( mapToGlobal(point) );
}

void ComponentSelector::slotManageComponents()
{
    m_pluginsdDialog.setPluginList();
    m_pluginsdDialog.setVisible( true );
}

void ComponentSelector::search( QString filter )
{
    QList<QTreeWidgetItem*> cList = findItems( filter, Qt::MatchContains|Qt::MatchRecursive, 0 );
    QList<QTreeWidgetItem*> allItems = findItems( "", Qt::MatchContains|Qt::MatchRecursive, 0 );

    for( QTreeWidgetItem* item : allItems )
    {
        item->setHidden( true );

        if( item->childCount() > 0 ) continue;
        if( cList.contains( item ) )
        {
            item->setHidden( false );

            QTreeWidgetItem* parent = item->parent();
            while( parent )
            {
                parent->setHidden( false );
                parent = parent->parent();
}   }   }   }

#include "moc_componentselector.cpp"
