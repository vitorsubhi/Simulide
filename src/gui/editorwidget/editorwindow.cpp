/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QDomDocument>

#include "editorwindow.h"
#include "circuitwidget.h"
#include "mainwindow.h"
#include "mcu.h"
#include "simulator.h"
#include "compiler.h"
#include "utils.h"

#include "gcbdebugger.h"
#include "inodebugger.h"
#include "b16asmdebugger.h"
#include "avrgccdebugger.h"
#include "xc8debugger.h"
#include "sdccdebugger.h"
#include "picasmdebugger.h"

EditorWindow* EditorWindow::m_pSelf = NULL;

EditorWindow::EditorWindow( QWidget* parent )
            : EditorWidget( parent )
{
    m_pSelf = this;

    m_debugDoc = NULL;
    m_debugger = NULL;

    m_state     = DBG_STOPPED;
    m_driveCirc = true;
    m_stepOver  = false;

    CodeEditor::readSettings();
    loadCompilers();
}
EditorWindow::~EditorWindow(){}

void EditorWindow::updateStep()
{
    if( !m_updateScreen ) return;

    m_debugDoc->updateScreen();
    m_updateScreen = false;
}

bool EditorWindow::upload()
{
    return uploadFirmware( false );
}

bool EditorWindow::uploadFirmware( bool debug )
{
    CodeEditor* ce = getCodeEditor();
    if( !ce ) return false;

    bool ok = ce->compile( debug );
    if( ok ) ok = ce->getCompiler()->upload();

    return ok;
}

void EditorWindow::debug()
{
    if( initDebbuger() )
    {
        m_editorToolBar->setVisible( false );
        m_compileToolBar->setVisible( false );
        m_debuggerToolBar->setVisible( true );

        runAct->setEnabled( true );
        stepAct->setEnabled( true );
        stepOverAct->setEnabled( true );
        resetAct->setEnabled( true );
        pauseAct->setEnabled( false );
}   }

void EditorWindow::run()
{ 
    setStepActs();

    if( m_state == DBG_STOPPED ) return;
    m_state = DBG_RUNNING;
    m_debugger->run();
    if( m_driveCirc ) Simulator::self()->resumeSim();
}

void EditorWindow::step()
{ 
    setStepActs();
    stepDebug();
}

void EditorWindow::stepOver()
{
    setStepActs();
    stepDebug( true );
}

void EditorWindow::pause()
{
    if( m_state < DBG_STEPING )  return;
    if( m_driveCirc ) Simulator::self()->pauseSim();

    m_debugger->pause();
    m_updateScreen = true;

    m_resume = m_state;
    m_state  = DBG_PAUSED;

    runAct->setEnabled( true );
    stepAct->setEnabled( true );
    stepOverAct->setEnabled( true );
    resetAct->setEnabled( true );
    pauseAct->setEnabled( false );
}

void EditorWindow::reset()
{
    CircuitWidget::self()->powerCircOff();

    m_debugDoc->setDebugLine( 1 );
    m_debugDoc->updateScreen();

    m_lastCycle = 0;
    m_state = DBG_PAUSED;

    CircuitWidget::self()->powerCircDebug( m_driveCirc );
}

void EditorWindow::stop()
{ 
    stopDebbuger();
    m_outPane.appendLine( "\n"+tr("Debugger Stopped ")+"\n" );
    m_debuggerToolBar->setVisible( false );
    m_compileToolBar->setVisible( true );
    m_editorToolBar->setVisible( true);
}

bool EditorWindow::initDebbuger()
{
    m_outPane.appendLine( "-------------------------------------------------------\n" );
    m_outPane.appendLine( tr("Starting Debbuger...")+"\n" );

    m_debugDoc = NULL;
    m_debugger = NULL;
    m_state = DBG_STOPPED;
    bool ok = uploadFirmware( true );

    if( ok )  // OK: Start Debugging
    {
        m_debugDoc = getCodeEditor();
        m_debugger = m_debugDoc->getCompiler();
        m_debugDoc->startDebug();

        stepOverAct->setVisible( true /*m_stepOver*/ );
        eMcu::self()->setDebugging( true );
        ///reset();
        m_lastCycle = 0;
        m_lastTime = 0;
        m_state = DBG_PAUSED;
        m_debugDoc->setDebugLine( 1 );
        setDriveCirc( m_driveCirc );

        CircuitWidget::self()->powerCircDebug( m_driveCirc );
        Simulator::self()->addToUpdateList( this );

        m_outPane.appendLine("\n"+tr("Debugger Started")+"\n");
    }else{
        m_outPane.appendLine( "\n"+tr("Error Starting Debugger")+"\n" );
        stopDebbuger();
    }
    return ok;
}

void EditorWindow::stepDebug( bool over )
{
    if( m_state == DBG_RUNNING ) return;

    m_state = DBG_STEPING;
    m_debugger->stepFromLine( over );
    if( m_driveCirc ) Simulator::self()->resumeSim();

}

void EditorWindow::lineReached() // Processor reached PC related to source line
{
    pause();
    int cycle = eMcu::self()->cycle();
    double time = Simulator::self()->circTime()/1e6;
    m_outPane.appendLine( tr("Clock Cycles: ")+QString::number( cycle-m_lastCycle )
                          +"\t\t"+tr("Time us: ")+QString::number( time-m_lastTime ));
    m_lastCycle = cycle;
    m_lastTime = time;
    m_updateScreen = true;
}

void EditorWindow::stopDebbuger()
{
    if( m_state > DBG_STOPPED )
    {
        CircuitWidget::self()->powerCircOff();
        eMcu::self()->setDebugging( false );

        m_state = DBG_STOPPED;
        m_debugDoc->stopDebug();

        Simulator::self()->remFromUpdateList( this );
    }
}

void EditorWindow::setDriveCirc( bool drive )
{
    m_driveCirc = drive;

    if( m_state == DBG_PAUSED )
    { if( drive ) Simulator::self()->pauseSim(); }
}

BaseDebugger* EditorWindow::createDebugger( QString name, CodeEditor* ce , QString code )
{
    BaseDebugger* debugger = NULL;
    QString type = m_compilers.value( name ).type;
    QString file = m_compilers.value( name ).file;
    if( type.isEmpty() )
    {
        type = m_assemblers.value( name ).type;
        file = m_assemblers.value( name ).file;
    }
    if     ( type == "arduino")  debugger = new InoDebugger( ce, &m_outPane );
    else if( type == "avrgcc" )  debugger = new AvrGccDebugger( ce, &m_outPane );
    else if( type == "xc8" )     debugger = new Xc8Debugger( ce, &m_outPane );
    else if( type == "sdcc" )    debugger = new SdccDebugger( ce, &m_outPane );
    else if( type == "gcbasic" ) debugger = new GcbDebugger( ce, &m_outPane );

    //else if( type == "gputils" ) debugger = new PicAsmDebugger( ce, &m_outPane );
    //else if( type == "b16asm" )  debugger = new B16AsmDebugger( ce, &m_outPane );
    else
    {
        debugger = new BaseDebugger( ce, &m_outPane );
        if( name != "None" ) code = type.right( 2 );
        debugger->setLstType( code.right( 1 ).toInt() );
        debugger->setLangLevel( code.left( 1 ).toInt() );
    }
    if( name != "None" ) debugger->loadCompiler( file );
    return debugger;
}

void EditorWindow::loadCompilers()
{
    QString compilsPath = MainWindow::self()->getFilePath("data/codeeditor/compilers/compilers");
    loadCompilerSet( compilsPath, &m_compilers );
    compilsPath = MainWindow::self()->getFilePath("data/codeeditor/compilers/assemblers");
    loadCompilerSet( compilsPath, &m_assemblers );
}

void EditorWindow::loadCompilerSet( QString compilsPath, QMap<QString, compilData_t>* compList )
{
    QDir compilsDir = QDir( compilsPath );

    compilsDir.setNameFilters( QStringList( "*.xml" ) );

    QStringList xmlList = compilsDir.entryList( QDir::Files, QDir::Name );
    if( xmlList.isEmpty() ) return;                  // No compilers to load
    qDebug() <<"\n"<<tr("    Loading Compilers at:")<<"\n"<<compilsPath<<"\n";

    for( QString compilFile : xmlList )
    {
        QString compilFilePath = compilsDir.absoluteFilePath( compilFile );
        if( !compilFilePath.isEmpty( ))
        {
            QDomDocument domDoc = fileToDomDoc( compilFilePath, "EditorWindow::loadCompilers");
            if( domDoc.isNull() ) continue;

            QDomElement el = domDoc.documentElement();
            if( (el.tagName() == "compiler")
              && el.hasAttribute( "name" )
              && el.hasAttribute( "type" )  )
            {
                QString compiler = el.attribute( "name" ) ;

                compilData_t compilData;
                compilData.file = compilFilePath;
                compilData.type = el.attribute( "type" ) ;

                compList->insert( compiler, compilData );
                qDebug() << tr("        Found Compiler: ") << compiler;
            }
            else qDebug() << tr("Error Loading Compiler at:") <<"\n" << compilFilePath <<"\n";
}   }   }

#include  "moc_editorwindow.cpp"
