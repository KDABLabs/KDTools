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

#ifndef COMMON_H
#define COMMON_H

#include <QDir>
#include <QString>
#include <QFile>

#include <stdlib.h>


static bool cleanAppDir()
{
    // Delete application directory
    QDir appDir( QFile::decodeName( EXAMPLE_APP_DIR ) + QLatin1String( "_tmp" ) );
    QString command;
    if ( appDir.exists() ) {
#ifdef Q_OS_WIN32
        command = QString::fromLatin1( "del /S /Q %1" ).arg( appDir.path() );
#else
        command = QString::fromLatin1( "rm -rf %1" ).arg( appDir.path() );
#endif
        qDebug( "%s", command.toLatin1().constData() );
        const int ret = system( command.toLatin1().constData() );
        return ret == 0;
    }
    return false;
}

static bool restoreAppDir() {
    // Restore application directory
    cleanAppDir();
    QString command;
    QDir appDir( QFile::decodeName( EXAMPLE_APP_DIR ) + QLatin1String( "_tmp" ) );
    QDir backupDir( QFile::decodeName( EXAMPLE_APP_DIR ) );
    if ( backupDir.exists() ) {
#ifdef Q_OS_WIN32
        command = QString::fromLatin1( "xcopy /E /I /Y %1 %2" ).arg( backupDir.path(), appDir.path() );
#else
        command = QString::fromLatin1( "cp -r %1 %2" ).arg( backupDir.path(), appDir.path() );
#endif
        qDebug( "%s", command.toLatin1().constData() );
        const int ret = system( command.toLatin1().constData() );
        return ret == 0;
    }
    return false;
}

class AppDirSaver {
public:
    AppDirSaver() { restoreAppDir(); }
    ~AppDirSaver() { cleanAppDir(); }
};

#endif /* COMMON_H */

