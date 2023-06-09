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
#include <QSettings>
#include <QStandardPaths>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QSplitter>
#include <QLineEdit>
#include <QTextStream>
#include <QDebug>
#include <QPluginLoader>
#include <QStyleFactory>
#include <QProcessEnvironment>

#include "mainwindow.h"
#include "circuit.h"
#include "componentselector.h"
#include "editorwindow.h"
#include "circuitwidget.h"
#include "filewidget.h"
#include "utils.h"

MainWindow* MainWindow::m_pSelf = NULL;

MainWindow::MainWindow()
          : QMainWindow()
{
    setWindowIcon( QIcon(":/simulide.png") );
    m_pSelf   = this;
    m_circuit = NULL;
    m_autoBck = 15;
    m_state = "■";
    m_version = "SimulIDE-"+QString( APP_VERSION )+" R"+QString( REVNO );
    
    this->setWindowTitle( m_version );

    QString appImg = QProcessEnvironment::systemEnvironment().value( QStringLiteral("APPIMAGE") );
    if( !appImg.isEmpty() ) m_filesDir.setPath( appImg.left( appImg.lastIndexOf("/") ) );
    else                    m_filesDir.setPath( QApplication::applicationDirPath() );

    if( m_filesDir.exists("../share/simulide") ) m_filesDir.cd("../share/simulide");

    m_configDir.setPath( QStandardPaths::writableLocation( QStandardPaths::DataLocation ) );

    m_settings = new QSettings( getConfigPath("simulide.ini"), QSettings::IniFormat, this );

    QString userAddonPath = getConfigPath("addons");
    QDir pluginsDir( userAddonPath );
    if( !pluginsDir.exists() ) pluginsDir.mkpath( userAddonPath );

    // Fonts --------------------------------------
    QFontDatabase::addApplicationFont( ":/Ubuntu-B.ttf" );
    QFontDatabase::addApplicationFont( ":/UbuntuMono-B.ttf" );
    QFontDatabase::addApplicationFont( ":/UbuntuMono-BI.ttf" );
    QFontDatabase::addApplicationFont( ":/UbuntuMono-R.ttf" );
    QFontDatabase::addApplicationFont( ":/UbuntuMono-RI.ttf" );

    float scale = 1.0;
    if( m_settings->contains( "fontScale" ) )
    {
        scale = m_settings->value( "fontScale" ).toFloat();
    }else{
        float dpiX = qApp->desktop()->logicalDpiX();
        scale = dpiX/96.0;
    }
    setFontScale( scale );
    //----------------------------------------------

    QApplication::setStyle( QStyleFactory::create("Fusion") ); //applyStyle();
    createWidgets();

    QDir compSetDir = m_filesDir.absoluteFilePath("data");
    if( compSetDir.exists() ) ComponentSelector::self()->LoadCompSetAt( compSetDir );
    readSettings();

    QString backPath = getConfigPath( "backup.sim1" );
    if( QFile::exists( backPath ) )
    {
        QMessageBox msgBox;
        msgBox.setText( tr("Looks like SimulIDE crashed...")+"\n\n"
                       +tr("There is an auto-saved copy of the Circuit\n")
                       +tr("You must save it with any other name if you want to keep it")+"\n\n"
                       +tr("This file will be auto-deleted!!" )+"\n");
        msgBox.setInformativeText(tr("Do you want to open the auto-saved copy of the Circuit?"));
        msgBox.setStandardButtons( QMessageBox::Open | QMessageBox::Discard );
        msgBox.setDefaultButton( QMessageBox::Open );
        if( msgBox.exec() == QMessageBox::Open )
             CircuitWidget::self()->loadCirc( backPath );
        else QFile::remove( backPath ); // Remove backup file
    }
}
MainWindow::~MainWindow(){ }

void MainWindow::closeEvent( QCloseEvent *event )
{
    if( !m_editor->close() )      { event->ignore(); return; }
    if( !m_circuit->newCircuit()) { event->ignore(); return; }
    
    writeSettings();
    event->accept();
}

void MainWindow::readSettings()
{
    restoreGeometry(                 m_settings->value( "geometry" ).toByteArray());
    restoreState(                    m_settings->value( "windowState" ).toByteArray());
    m_Centralsplitter->restoreState( m_settings->value( "Centralsplitter/geometry" ).toByteArray());
    CircuitWidget::self()->splitter()->restoreState( m_settings->value( "Circsplitter/geometry" ).toByteArray());

    m_autoBck = 15;
    if( m_settings->contains( "autoBck" )) m_autoBck = m_settings->value( "autoBck" ).toInt();
    Circuit::self()->setAutoBck( m_autoBck );
}

void MainWindow::writeSettings()
{
    m_settings->setValue( "autoBck",   m_autoBck );
    m_settings->setValue( "fontScale", m_fontScale );
    m_settings->setValue( "geometry",  saveGeometry() );
    m_settings->setValue( "windowState", saveState() );
    m_settings->setValue( "Centralsplitter/geometry", m_Centralsplitter->saveState() );
    m_settings->setValue( "Circsplitter/geometry", CircuitWidget::self()->splitter()->saveState() );
    
    QList<QTreeWidgetItem*> list = m_components->findItems( "", Qt::MatchStartsWith | Qt::MatchRecursive );

    for( QTreeWidgetItem* item : list  )
        m_settings->setValue( item->text(0)+"/collapsed", !item->isExpanded() );

    FileWidget::self()->writeSettings();
}

void MainWindow::setFontScale(float scale )
{
    if     ( scale < 0.5 ) scale = 0.5;
    else if( scale > 2 )   scale = 2;
    m_fontScale = scale;
}

QString MainWindow::loc()
{
    if( m_lang == Czech )      return "cz";
    if( m_lang == French )     return "fr";
    if( m_lang == German )     return "de";
    if( m_lang == Italian )    return "it";
    if( m_lang == Russian )    return "ru";
    if( m_lang == Spanish )    return "es";
    if( m_lang == Portuguese ) return "pt_PT";
    if( m_lang == Pt_Brasil )  return "pt_BR";
    if( m_lang == Dutch )      return "nl";
    if( m_lang == Turkish )    return "tr";

    return "en";
}

void MainWindow::setLoc( QString loc )
{
    Langs lang = English;
    if     ( loc == "cz" )    lang = Czech;
    else if( loc == "fr" )    lang = French;
    else if( loc == "de" )    lang = German;
    else if( loc == "it" )    lang = Italian;
    else if( loc == "ru" )    lang = Russian;
    else if( loc == "es" )    lang = Spanish;
    else if( loc == "pt_PT" ) lang = Pt_Brasil;
    else if( loc == "pt_BR" ) lang = Pt_Brasil;
    else if( loc == "nl" )    lang = Dutch;
    else if( loc == "tr" )    lang = Turkish;

    m_lang = lang;
}

void MainWindow::setLang( Langs lang )
{
    m_lang = lang;
    settings()->setValue( "language", loc() );
}

void MainWindow::setFile( QString file )
{
    m_file = file;
    setWindowTitle( m_state+" "+m_version+" - "+file );
}

void MainWindow::setState( QString state )
{
    m_state = state;
    QString changed = windowTitle().endsWith("*") ? "*" : "";
    setWindowTitle( state+" "+m_version+" - "+m_file+changed );
}

void MainWindow::createWidgets()
{
    QWidget *centralWidget = new QWidget( this );
    setCentralWidget(centralWidget);

    QGridLayout *baseWidgetLayout = new QGridLayout( centralWidget );
    baseWidgetLayout->setSpacing(0);
    baseWidgetLayout->setContentsMargins(0, 0, 0, 0);

    m_Centralsplitter = new QSplitter( this );
    m_Centralsplitter->setOrientation( Qt::Horizontal );

    m_sidepanel = new QTabWidget( this );
    m_sidepanel->setTabPosition( QTabWidget::West );
    QString fontSize = QString::number( int(11*m_fontScale) );
    m_sidepanel->tabBar()->setStyleSheet("QTabBar { font-size:"+fontSize+"px; }");
    m_Centralsplitter->addWidget( m_sidepanel );

    m_componentWidget = new QWidget( this );
    m_componentWidgetLayout = new QVBoxLayout( m_componentWidget );
    m_componentWidgetLayout->setSpacing(0);
    m_componentWidgetLayout->setContentsMargins(0, 0, 0, 0);

    m_searchComponent = new QLineEdit( this );
    m_searchComponent->setPlaceholderText( tr( "Search Components" ));
    m_componentWidgetLayout->addWidget( m_searchComponent );
    connect( m_searchComponent, SIGNAL( editingFinished() ),
             this,               SLOT(  searchChanged()), Qt::UniqueConnection);

    m_components = new ComponentSelector( m_sidepanel );
    m_componentWidgetLayout->addWidget( m_components );

    m_sidepanel->addTab( m_componentWidget, tr("Components") );

    m_fileSystemTree = new FileWidget( this );
    m_sidepanel->addTab( m_fileSystemTree, tr( "File explorer" ) );

    m_circuit = new CircuitWidget( this );
    m_Centralsplitter->addWidget( m_circuit );
    
    m_editor = new EditorWindow( this );
    m_editor->setObjectName(QString::fromUtf8("editor"));
    m_Centralsplitter->addWidget( m_editor );

    baseWidgetLayout->addWidget( m_Centralsplitter, 0, 0 );

    QList<int> sizes;
    sizes << 150 << 350 << 500;
    m_Centralsplitter->setSizes( sizes );

    this->showMaximized();
}

void MainWindow::searchChanged()
{
    QString filter = m_searchComponent->text();
    m_components->search( filter );
}

QString MainWindow::getHelp( QString name )
{
    QString help = tr("No help available");

    if( m_help.contains( name ) ) return m_help.value( name );

    QString locale = loc();
    QString localeFolder = "";

    if( loc() != "en" ) {
        locale.prepend("_");
        localeFolder = locale + "/";
    }
    else locale = "";

    name= name.toLower().replace( " ", "" );
    QString dfPath = getFilePath("data/help/"+localeFolder+name+locale+".txt");

    if( dfPath == "" ) dfPath = getFilePath( "data/help/"+name+".txt" );
    if( dfPath != "" )
    {
        QFile file( dfPath );

        if( file.open( QFile::ReadOnly | QFile::Text) ) // Get Text from Help File
        {
            QTextStream s1( &file );
            s1.setCodec("UTF-8");
            help = s1.readAll();
            file.close();
        }
        else qDebug() << "Warning: MainWindow::getHelp: File not found\n"<<dfPath;
    }
    m_help[name] = help;
    return help;
}

QSettings* MainWindow::settings() { return m_settings; }

QString MainWindow::getFilePath( QString file )   { return m_filesDir.absoluteFilePath( file ); }
QString MainWindow::getConfigPath( QString file ) { return m_configDir.absoluteFilePath( file ); }

#include  "moc_mainwindow.cpp"
