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
#include <QTextStream>

#include <QScriptEngine>
#include <QScriptValue>


MainWindow::MainWindow( KDUpdater::Target * target, QWidget * parent )
    : QMainWindow(parent),
      m_target( target ),
      m_lastId(0),
      m_appDir( QFile::decodeName( EXAMPLE_APP_DIR ) )
{
    m_appDir.mkpath( QLatin1String("objects") );
    m_objectsDir = m_appDir;
    m_objectsDir.cd( QLatin1String("objects") );

    QToolBar *bar = addToolBar(tr("Actions"));
    bar->addAction(tr("Add Object 1"), this, SLOT(onAdd1()));
    bar->addAction(tr("Add Object 2"), this, SLOT(onAdd2()));
    bar->addAction(tr("Update Objects"), this, SLOT(onUpdateSelected()));
    bar->addAction( tr( "Update" ), this, SLOT(onUpdate()) );
    bar->addAction( tr( "Update Compat" ), this, SLOT(onUpdateCompat()) );

    m_table = new QTableWidget(this);
    m_table->verticalHeader()->hide();
    m_table->setSelectionBehavior(QTableWidget::SelectRows);
    setCentralWidget(m_table);

    loadApplicationData();

    QStringList devices = m_objectsDir.entryList();
    Q_FOREACH( const QString & object, devices ) {
        if ( object.contains( QLatin1Char('.') ) ) continue;

        bool ok;
        int id = object.toInt(&ok);

        if (!ok) continue;

        if (id>m_lastId) m_lastId = id;

        int row = m_table->rowCount();
        m_table->insertRow(m_table->rowCount());
        displayObject(id, row);
    }
}

MainWindow::~MainWindow()
{
}

void MainWindow::loadApplicationData()
{
    // Loading the headers
    QFile headersFile( m_appDir.absoluteFilePath( QLatin1String("headers.js") ) );
    headersFile.open(QFile::ReadOnly);
    const QString script = QTextStream( &headersFile ).readAll();
    headersFile.close();

    QScriptEngine engine;
    engine.evaluate(script);
    QScriptValue function = engine.globalObject().property( QLatin1String("headers") );

    QStringList headers = function.call().toString().split( QLatin1String(", ") );
    m_table->setColumnCount(headers.size());
    m_table->setHorizontalHeaderLabels(headers);

    // Loading the stylesheet
    if ( m_appDir.exists( QLatin1String("application.qss") ) ) {
        QFile styleFile( m_appDir.absoluteFilePath( QLatin1String("application.qss") ) );
        styleFile.open(QFile::ReadOnly);
        const QString stylesheet = QTextStream( &styleFile ).readAll();
        styleFile.close();
        qApp->setStyleSheet(stylesheet);
    }
    else {
        qApp->setStyleSheet( QString() );
    }
}

void MainWindow::updateObject(const QString &objectName, int object)
{
    QFile objectFile(m_objectsDir.absoluteFilePath(QString::number(object)));
    QFile file(m_appDir.absoluteFilePath(objectName));

    objectFile.remove();
    QFile::copy(file.fileName(), objectFile.fileName());
}

void MainWindow::displayObject(int object, int row)
{
    QFile objectFile(m_objectsDir.absoluteFilePath(QString::number(object)));
    QStringList objectText;

    objectFile.open(QFile::ReadOnly);
    QTextStream ts( &objectFile );
    while( !ts.atEnd() )
        objectText << ts.readLine()
            .remove(QLatin1Char('\r'))
            .remove(QLatin1Char('\n'));

    // Make it crash on purpose if the object has too many lines
    // simulates incompatibilities
    Q_ASSERT(objectText.size()<=m_table->columnCount()-1);

    m_table->setItem(row, 0, new QTableWidgetItem(QString::number(object)));

    for (int i = 0; i<objectText.size(); ++i) {
        m_table->setItem(row, i+1, new QTableWidgetItem(objectText[i]));
    }
}

void MainWindow::onAdd1()
{
    int id = ++m_lastId;
    updateObject( QLatin1String("object1.txt"), id );

    int row = m_table->rowCount();
    m_table->insertRow(m_table->rowCount());
    displayObject(id, row);
}

void MainWindow::onAdd2()
{
    int id = ++m_lastId;
    updateObject( QLatin1String("object2.txt"), id );

    int row = m_table->rowCount();
    m_table->insertRow(m_table->rowCount());
    displayObject(id, row);
}

void MainWindow::onUpdateSelected()
{
    QList <QTableWidgetItem*> items = m_table->selectedItems();
    QSet<int> rows;

    Q_FOREACH( QTableWidgetItem * item, items )
        rows << item->row();

    Q_FOREACH( int row, rows ) {
        int id = m_table->item(row, 0)->data(Qt::DisplayRole).toInt();

        QString type = m_table->item(row, 1)->data(Qt::DisplayRole).toString();
        if ( type == QLatin1String("Oject1") ) {
            updateObject( QLatin1String("object1.txt"), id );
        } else {
            updateObject( QLatin1String("object2.txt"), id );
        }

        displayObject(id, row);
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
