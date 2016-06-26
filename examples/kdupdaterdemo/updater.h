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

#ifndef UPDATER_H
#define UPDATER_H

#include <KDToolsCore/pimpl_ptr>
#include <QObject>

QT_BEGIN_NAMESPACE
class QString;
QT_END_NAMESPACE

class Updater : public QObject
{
    Q_OBJECT
public:    
    Updater();
    ~Updater();
    QString getInstalledAppVersion() const;
    QString getAppName() const;
    QString getUpdateVersion() const;
    bool hasUpdates() const;
    bool checkForUpdates();
    bool installUpdates();

private:
    class Private;
    kdtools::pimpl_ptr< Private > d;
};

#endif // UPDATER_H
