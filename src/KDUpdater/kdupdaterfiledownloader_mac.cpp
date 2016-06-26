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

#include <QtCore/QEventLoop>

#ifdef QT_MAC_USE_COCOA

/*!
 This encourages Qt to start a NSApp if there's none running yet. This makes sure
 that Qt is handling meta call events properly. Background: downloader calls doDownload as
 queued connection, this gets not called for the first time if there's no QCoreApplication
 being exec()'d yet. This is only required if Qt is using Cocoa.
 \internal
 */
void kdupdaterEncourageQtToStartNSApp()
{
    static bool didEncourage = false;
    if( !didEncourage )
    {
        QEventLoop loop;
        QMetaObject::invokeMethod(&loop, "quit", Qt::QueuedConnection);
        loop.exec(QEventLoop::ExcludeUserInputEvents|QEventLoop::ExcludeUserInputEvents);
        didEncourage = true;
    }
}

#endif // QT_MAC_USE_COCOA
