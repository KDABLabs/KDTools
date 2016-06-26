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

#include <QtGui/QApplication>
#include <QtGui/QMessageBox>

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QProcess>
#include <QtCore/QTimer>

#include <KDUpdater/Application>

#include <algorithm>
#include <functional>

#include "mainwindow.h"

bool copyUpdaterfiles( const QString& targetDir );
const QString copyFlagFileName = QLatin1String( "directoryWasCopied" );
#include <QDebug>
int main( int argc, char** argv )
{

    QApplication app( argc, argv );
    Q_INIT_RESOURCE( kdupdaterdemo );
    const QDir appDir( QApplication::applicationDirPath() );
#ifdef Q_OS_MAC
    const QDir backupDir( appDir.absoluteFilePath( QLatin1String( "../../../kdupdaterdemobackup" ) ) );
#else
    const QDir backupDir( appDir.absoluteFilePath( QLatin1String( "kdupdaterdemobackup" ) ) );
#endif
    const QStringList&  arguments = app.arguments();
    const QFileInfo copyFlagFileInfo( appDir.absoluteFilePath( copyFlagFileName ) );
    const QString copyOpt = QString::fromLatin1( "--copy" );
    const QString removeOpt = QString::fromLatin1( "--remove" );
    QStringList::const_iterator it = std::find( arguments.constBegin(), arguments.constEnd(), copyOpt );
    if ( it != arguments.end() )
    {
        ++it;
        if ( it != arguments.constEnd() )
        {
            QTimer::singleShot( 500, &app, SLOT(quit()) );
            app.exec();
            const QString fileToOverwrite = *it;
            const bool successfullRemove = QFile::remove( fileToOverwrite );
            if ( !successfullRemove )
                QMessageBox::critical( NULL, QString::fromLatin1( "Error replacing file" ),
                                       QString::fromLatin1( "Error the file %1 could not be removed!" ).arg( fileToOverwrite ),
                                       QMessageBox::Ok, QMessageBox::Ok );
            const bool successfullCopy = QFile::copy( qApp->applicationFilePath(), fileToOverwrite );
            if ( !successfullCopy )
                QMessageBox::critical( NULL, QString::fromLatin1( "Error replacing file" ),
                                       QString::fromLatin1( "Error the file %1 could not be written!" ).arg( fileToOverwrite ),
                                       QMessageBox::Ok, QMessageBox::Ok );
            QStringList processArgs;
            processArgs.append( removeOpt );
            processArgs.append( qApp->applicationFilePath() );
            QProcess::startDetached( fileToOverwrite, processArgs );
            return 0;
        }
    }
    it = std::find( arguments.constBegin(), arguments.constEnd(), removeOpt );
    if ( it != arguments.end() )
    {
        ++it;
        if ( it != arguments.constEnd() )
        {
            QTimer::singleShot( 500, &app, SLOT(quit()) );
            app.exec();
            const QString fileToDelete = *it;
            QFile::remove( fileToDelete );
        }
    }

    QString demoOriginDir;
    bool close = false;
    if ( copyFlagFileInfo.exists() )
    {
        QFile copyFlagFile( copyFlagFileInfo.absoluteFilePath() );
        bool opened = copyFlagFile.open( QFile::ReadOnly );
        if ( !opened )
        {
            QMessageBox::critical( NULL, QObject::tr( "OpenFileError"), copyFlagFile.errorString(), QMessageBox::Ok );
            copyFlagFile.unsetError();
            return false;
        }
        demoOriginDir = QString::fromUtf8( copyFlagFile.readAll().constData() );
    }
    if ( !arguments.contains( QLatin1String( "--no-clone" ) ) && !copyFlagFileInfo.exists() )
    {
        const QMessageBox::Button answer = QMessageBox::question( NULL, QObject::tr( "Backup" ),
                                                                  QObject::tr(
                                                                      "This demo will replace itself during the update process.\n"
                                                                      "If you want to reuse this demo it is strongly recommended\n"
                                                                      "to make a backup of this demo.\n"
                                                                      "Shall a backup be created in %1 ?\n"
                                                                      "If you do, this demo will ask you on close if it should\n"
                                                                      "dispose itself."
                                                                      ).arg( backupDir.absolutePath() ), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes );
        if ( answer == QMessageBox::Yes)
        {
            copyUpdaterfiles( backupDir.absolutePath() );
            QProcess::startDetached( backupDir.absoluteFilePath( QFileInfo ( QApplication::applicationFilePath() ).fileName() ), QStringList(), backupDir.absolutePath() );
            close = true;
        }

    }
    int retval = 0;
    if ( !close )
    {
        MainWindow window;
        window.resize( 800, 600 );
        window.show();
        retval = app.exec();

    }
    return retval;
}

/*!
  creates a backup of the updaterdemo and a file that marks this copy as a copy
 */
bool copyUpdaterfiles( const QString& targetDir )
{
    QDir dir( targetDir );
    if ( dir.exists() )
    {
        const QMessageBox::Button answer = QMessageBox::question( NULL, QObject::tr( "Override directory" ),
                                                                  QObject::tr(
                                                                      "The directory %1 already exists do you want to override files.\n"
                                                                      "in it?\n"
                                                                      ).arg( dir.absolutePath() ), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes );
        if ( answer == QMessageBox::No)
        {
            return false;
        }
    }
    else
    {
        bool created = QDir::current().mkdir( dir.path() );
        if ( !created )
        {
            QMessageBox::critical( NULL, QObject::tr( "CreateDirectoryError" ),
                                   QObject::tr( "The directory %1 could not be created" ).arg( dir.absolutePath() ),
                                   QMessageBox::Ok );
            return false;
        }
    }
    const QFileInfo applicationFileInfo( QApplication::applicationFilePath() );
    QFile applicationFile( QApplication::applicationFilePath() );
    bool copied = applicationFile.copy( dir.absoluteFilePath( applicationFileInfo.fileName() ) );
    if ( !copied )
    {
        QMessageBox::critical( NULL, QObject::tr( "CopyError"), applicationFile.errorString(), QMessageBox::Ok );
        applicationFile.unsetError();
        return false;
    }
    QFile copyFlagFile( dir.absoluteFilePath( copyFlagFileName ) );
    bool created = copyFlagFile.open( QFile::WriteOnly );
    if ( !created )
    {
        QMessageBox::critical( NULL, QObject::tr( "CreateFileError"), copyFlagFile.errorString(), QMessageBox::Ok );
        copyFlagFile.unsetError();
        QFile::remove( dir.absoluteFilePath( applicationFileInfo.fileName() ) );
        return false;
    }
    qint64 bytesWritten = copyFlagFile.write( applicationFileInfo.absoluteFilePath().toUtf8() );
    if ( bytesWritten == -1 )
    {
        QMessageBox::critical( NULL, QObject::tr( "WriteFileError"), copyFlagFile.errorString(), QMessageBox::Ok );
        copyFlagFile.unsetError();
        QFile::remove( dir.absoluteFilePath( applicationFileInfo.fileName() ) );
        return false;
    }
    copyFlagFile.flush();
    copyFlagFile.close();
    return true;
}
