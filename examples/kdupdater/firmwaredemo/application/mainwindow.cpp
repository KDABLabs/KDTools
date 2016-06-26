/****************************************************************************
** Copyright (C) 2001-2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com.
** All rights reserved.
**
** This file is part of the KD Tools library.
**
** Licensees holding valid commercial KD Tools licenses may use this file in
** accordance with the KD Tools Commercial License Agreement provided with
** the Software.
**
** This file may be distributed and/or modified under the terms of the
** GNU Lesser General Public License version 2.1 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.LGPL.txt included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/

#include "mainwindow.h"
#include "helpdialog.h"

#include <KDUpdater/KDUpdater>
#include <KDUpdater/Update>
#include <KDUpdater/UpdatesDialog>
#include <KDUpdater/Target>

#include <QDir>
#include <QFile>

#include <QApplication>
#include <QToolBar>
#include <QTableWidget>
#include <QHeaderView>
#include <QDialog>
#include <QLayout>
#include <QDialogButtonBox>
#include <QProgressDialog>
#include <QStatusBar>
#include <QAction>
#include <QTextStream>

#include <QScriptEngine>
#include <QScriptValue>


MainWindow::MainWindow( KDUpdater::Target * target, QWidget * parent )
    : QMainWindow(parent),
      m_target( target ),
      m_lastId(0),
      m_appDir( QFile::decodeName( EXAMPLE_APP_DIR ) )
{
    m_appDir.mkpath( QLatin1String("devices") );
    m_devicesDir = m_appDir;
    m_devicesDir.cd( QLatin1String("devices") );

    QToolBar *bar = addToolBar(tr("Actions"));
    QAction* l4rAction = bar->addAction(tr("Add Line 4 Receiver"), this, SLOT(onAddL4R()));
    l4rAction->setToolTip( tr( "Create a new Line 4 Receiver item" ) );
    m_actionMap[QLatin1String("L4RAction")] = l4rAction;

    QAction* psmAction = bar->addAction(tr("Add PSM"), this, SLOT(onAddPSM()));
    psmAction->setToolTip( tr( "Create a new Personal Sound Monitor item" ) );
    m_actionMap[QLatin1String("PSMAction")] = psmAction;

    QAction* uploadAction = bar->addAction(tr("Upload Firmwares"), this, SLOT(onUploadSelected()));
    uploadAction->setToolTip( tr( "Upload current firmware to selected item" ) );
    m_actionMap[QLatin1String("UploadAction")] = uploadAction;

    QAction* updateAction = bar->addAction( tr( "Update" ), this, SLOT(onUpdate()) );
    updateAction->setToolTip( tr( "Display available updates and install them" ) );
    m_actionMap[QLatin1String("UpdateAction")] = updateAction;

    QAction* compatUpdateAction = bar->addAction( tr( "Update Compat" ), this, SLOT(onUpdateCompat()) );
    compatUpdateAction->setToolTip( tr( "Display available compat updates and\ninstall them (this will change the compat level)" ) );
    m_actionMap[QLatin1String("CompatUpdateAction")] = compatUpdateAction;

    /*bar->addAction("Packages", this, SLOT(showPackages()));*/
    QAction* sourcesAction = bar->addAction( tr("Update Sources"), this, SLOT(showSources()));
    m_actionMap[QLatin1String("SourcesAction")] = sourcesAction;

    m_table = new QTableWidget(this);
    m_table->verticalHeader()->hide();
    m_table->setSelectionBehavior(QTableWidget::SelectRows);

    QAction* selectAllAction = new QAction( this );
    connect( selectAllAction, SIGNAL(triggered()), m_table, SLOT(selectAll()) );
    m_actionMap[QLatin1String("SelectAllAction")] = selectAllAction;

    setCentralWidget(m_table);
    resize ( 640, 480 );

    statusBar();

    loadApplicationData();

    const QStringList devices = m_devicesDir.entryList();
    Q_FOREACH( const QString & device, devices ) {
        if ( device.contains( QLatin1Char('.') ) ) continue;

        bool ok;
        int id = device.toInt(&ok);

        if (!ok) continue;

        if (id>m_lastId) m_lastId = id;

        int row = m_table->rowCount();
        m_table->insertRow(m_table->rowCount());
        displayDevice(id, row);
    }

    HelpDialog* helpDialog = new HelpDialog( this );
    connect( helpDialog, SIGNAL(linkClicked(QString)),
             this, SLOT(triggerAction(QString)) );
    helpDialog->show();
}

MainWindow::~MainWindow()
{
}

void MainWindow::loadApplicationData()
{
    // Compat level
    statusBar()->showMessage( tr( "Current compat level: %1 - Current firmware version: %2" ).arg( m_target->compatLevel() ).arg( m_target->packagesInfo()->packageInfo( 0 ).version ) );

    // Loading the headers
    QFile headersFile( m_appDir.absoluteFilePath( QLatin1String("headers.js") ) );
    headersFile.open(QFile::ReadOnly);
    const QString script = QTextStream( &headersFile ).readAll();
    headersFile.close();

    QScriptEngine engine;
    engine.evaluate(script);
    QScriptValue function = engine.globalObject().property( QLatin1String("headers") );

    const QStringList headers = function.call().toString().split( QLatin1String(", ") );
    m_table->setColumnCount(headers.size());
    m_table->setHorizontalHeaderLabels(headers);

    // Loading the stylesheet
    if ( m_appDir.exists( QLatin1String( "application.qss" ) ) ) {
        QFile styleFile( m_appDir.absoluteFilePath( QLatin1String( "application.qss" ) ) );
        styleFile.open(QFile::ReadOnly);
        const QString stylesheet = QTextStream( &styleFile ).readAll();
        styleFile.close();
        qApp->setStyleSheet(stylesheet);
    }
    else {
        qApp->setStyleSheet( QString() );
    }
}

void MainWindow::uploadFirmware(const QString &firmware, int device)
{
    QFile devFile(m_devicesDir.absoluteFilePath(QString::number(device)));
    QFile firmwareFile(m_appDir.absoluteFilePath(firmware));

    devFile.remove();
    QFile::copy(firmwareFile.fileName(), devFile.fileName());
}

void MainWindow::displayDevice(int device, int row)
{
    QFile devFile(m_devicesDir.absoluteFilePath(QString::number(device)));
    QStringList firmware;

    devFile.open(QFile::ReadOnly);
    QTextStream ts( &devFile );
    while( !ts.atEnd() )
        firmware << ts.readLine()
                    .remove(QLatin1Char('\r'))
                    .remove(QLatin1Char('\n'));

    // Make it crash on purpose if the firmware has too many lines
    // simulates incompatibilities
    Q_ASSERT(firmware.size()<=m_table->columnCount()-1);

    m_table->setItem(row, 0, new QTableWidgetItem(QString::number(device)));

    for (int i = 0; i<firmware.size(); ++i) {
        m_table->setItem(row, i+1, new QTableWidgetItem(firmware[i]));
    }
}

void MainWindow::onAddL4R()
{
    int id = ++m_lastId;
    uploadFirmware( QLatin1String("l4rfirmware.txt"), id );

    int row = m_table->rowCount();
    m_table->insertRow(m_table->rowCount());
    displayDevice(id, row);
}

void MainWindow::onAddPSM()
{
    int id = ++m_lastId;
    uploadFirmware( QLatin1String("psmfirmware.txt"), id );

    int row = m_table->rowCount();
    m_table->insertRow(m_table->rowCount());
    displayDevice(id, row);
}

void MainWindow::onUploadSelected()
{
    QList <QTableWidgetItem*> items = m_table->selectedItems();
    QSet<int> rows;

    Q_FOREACH( QTableWidgetItem * item, items )
        rows << item->row();

    Q_FOREACH( int row, rows ) {
        int id = m_table->item(row, 0)->data(Qt::DisplayRole).toInt();

        QString type = m_table->item(row, 1)->data(Qt::DisplayRole).toString();
        if ( type == QLatin1String("Line 4 Receiver") ) {
            uploadFirmware( QLatin1String("l4rfirmware.txt"), id );
        } else {
            uploadFirmware( QLatin1String("psmfirmware.txt"), id );
        }

        displayDevice(id, row);
    }
}

void MainWindow::onUpdate()
{
    updatePackage( false );
}

void MainWindow::onUpdateCompat()
{
    updatePackage( true );
}

void MainWindow::updatePackage( bool findCompat )
{
    QProgressDialog dialog(this);

    // Compute updates
    KDUpdater::UpdateFinder updateFinder( m_target );
    connect(&updateFinder, SIGNAL(progressValue(int)), &dialog, SLOT(setValue(int)));
    connect(&updateFinder, SIGNAL(progressText(QString)), &dialog, SLOT(setLabelText(QString)));
    if ( findCompat )
        updateFinder.setUpdateType(KDUpdater::CompatUpdate);
    updateFinder.run();

    // Get updates
    QList<KDUpdater::Update*> updates = updateFinder.updates();
    dialog.hide();
    disconnect(&updateFinder, 0, &dialog, 0);

    KDUpdater::UpdatesDialog updatesDialog(this);
    updatesDialog.setUpdates(updates);

    if( updatesDialog.exec() != QDialog::Accepted )
        return;

    QList<KDUpdater::Update*> reqUpdates;
    for(int i=0; i<updates.count(); i++)
    {
        if( !updatesDialog.isUpdateAllowed(updates[i]) )
            continue;
        reqUpdates.append(updates[i]);
    }

    // Install updates
    KDUpdater::UpdateInstaller updateInstaller( m_target );
    connect(&updateInstaller, SIGNAL(progressValue(int)), &dialog, SLOT(setValue(int)));
    connect(&updateInstaller, SIGNAL(progressText(QString)), &dialog, SLOT(setLabelText(QString)));
    dialog.show();
    updateInstaller.setUpdatesToInstall(reqUpdates);
    updateInstaller.run();

    // Show the updated thing
    loadApplicationData();
}

void MainWindow::triggerAction( const QString& name )
{
    if ( m_actionMap.contains( name ) )
        m_actionMap.value( name )->trigger();
}

void MainWindow::showPackages()
{
    KDUpdater::PackagesView* pView = new KDUpdater::PackagesView( this );
    pView->setPackageInfo( m_target->packagesInfo() );

    // Code used to display the widget as a dialog
    pView->setWindowTitle( tr("Installed Packages") );
    pView->setWindowFlags( Qt::Dialog );
    pView->setAttribute( Qt::WA_DeleteOnClose, true );
    pView->resize(320, 240);
    pView->show();
}

void MainWindow::showSources()
{
    KDUpdater::UpdateSourcesView* usView = new KDUpdater::UpdateSourcesView;
    usView->setUpdateSourcesInfo( m_target->updateSourcesInfo() );

    // Code used to display the widget as a dialog
    usView->setWindowTitle( tr("Update Sources") );
    usView->setWindowFlags( Qt::Dialog );
    usView->setAttribute( Qt::WA_DeleteOnClose, true );
    usView->resize(320, 240);
    usView->show();
}
