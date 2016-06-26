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

#include "windowsrunningapplicationupdateoperation.h"
#include <QtCore/QProcess>
#include <QtCore/QStringList>
#include <QtCore/QFileInfo>
#include <QtCore/QCoreApplication>

WindowsRunningApplicationUpdateOperation::WindowsRunningApplicationUpdateOperation()
{
}

WindowsRunningApplicationUpdateOperation::~WindowsRunningApplicationUpdateOperation()
{
}


void WindowsRunningApplicationUpdateOperation::backup()
{
}

bool WindowsRunningApplicationUpdateOperation::performOperation()
{    
    QStringList processArgs;
    Q_FOREACH( const QString & arg, arguments() )
    {
        QFileInfo fi( arg );
        processArgs.append( fi.absoluteFilePath() );
        processArgs.append( fi.fileName() );
    }
    processArgs.append( QString::number( QCoreApplication::applicationPid() ) );

    QProcess::startDetached( QLatin1String( "UpdateHelper"), processArgs, qApp->applicationDirPath() );
    return true;
}

bool WindowsRunningApplicationUpdateOperation::undoOperation()
{
    return true;
}
bool WindowsRunningApplicationUpdateOperation::testOperation()
{
    return true;
}

KDUpdater::UpdateOperation* WindowsRunningApplicationUpdateOperation::clone() const
{
    return new WindowsRunningApplicationUpdateOperation();
}
