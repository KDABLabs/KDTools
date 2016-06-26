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
#include "ui_mainwindow.h"

#include <KDUpdater/KDUpdater>

#include <QToolBar>
#include <QAction>
#include <QDir>
#include <QFile>
#include <QDialog>
#include <QDialogButtonBox>
#include <QProgressDialog>
#include <QLabel>
#include <QFileInfo>
#include <QTextStream>


MainWindow::MainWindow( KDUpdater::Target * target, QWidget * parent )
    : QMainWindow(parent),
      ui( new Ui::MainWindow ),
      m_target( target )
{
    ui->setupUi(this);

    QToolBar* updateTools = addToolBar( tr("Update Tools") );
    updateTools->addAction( tr("Update"),
                           this, SLOT(updatePackages()));
    updateTools->addAction( tr("Packages"),
                           this, SLOT(showPackages()));
    updateTools->addAction( tr("Update Sources"),
                           this, SLOT(showSources()));

    init();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::init()
{
    // Clear old data
    ui->iconLabel->clear();
    ui->textEdit->clear();

    // Get the target directory
    QString targetDirPath = m_target->directory();
    targetDirPath = QFileInfo( targetDirPath ).absoluteFilePath();

    // Read the icon
    QFileInfo iconInfo( targetDirPath + QLatin1String("/icon.png") );
    if ( iconInfo.exists() ) {
        ui->iconLabel->setPixmap( QPixmap( iconInfo.filePath() ) );
    }

    // Read the license
    QFileInfo licenseInfo( targetDirPath + QLatin1String("/license.txt") );
    if ( licenseInfo.exists() ) {
        QFile file( licenseInfo.filePath() );
        if( !file.open(QFile::ReadOnly) )
            return;
        const QString fileStr = QTextStream( &file ).readAll();
        ui->textEdit->setPlainText( fileStr );
    }

    // Show the target directory in the mainwindow's titlebar
    setWindowTitle( QString::fromLatin1("%1 - %2").arg( m_target->name(), targetDirPath ) );
}

void MainWindow::updatePackages()
{
    QProgressDialog* dialog = new QProgressDialog(this);

    // Compute updates
    KDUpdater::UpdateFinder updateFinder( m_target );
    connect(&updateFinder, SIGNAL(progressValue(int)), dialog, SLOT(setValue(int)));
    connect(&updateFinder, SIGNAL(progressText(QString)), dialog, SLOT(setLabelText(QString)));
    updateFinder.run();

    // Get updates
    QList<KDUpdater::Update*> updates = updateFinder.updates();
    dialog->hide();
    disconnect(&updateFinder, 0, dialog, 0);

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
    connect(&updateInstaller, SIGNAL(progressValue(int)), dialog, SLOT(setValue(int)));
    connect(&updateInstaller, SIGNAL(progressText(QString)), dialog, SLOT(setLabelText(QString)));
    dialog->show();
    updateInstaller.setUpdatesToInstall(reqUpdates);
    updateInstaller.run();

    delete dialog;

    // Show the updated thing
    init();
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
