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

#ifndef __KDTOOLS_KDUPDATERTARGET_H__
#define __KDTOOLS_KDUPDATERTARGET_H__

#include "kdupdater.h"
#include <KDToolsCore/pimpl_ptr.h>

QT_BEGIN_NAMESPACE
class QUrl;
QT_END_NAMESPACE

namespace KDUpdater
{
    class PackagesInfo;
    class UpdateSourcesInfo;
    class Task;

    class KDTOOLS_UPDATER_EXPORT Target {
        DOXYGEN_PROPERTY( QString directory READ directory WRITE setDirectory )
        DOXYGEN_PROPERTY( QString name READ name )
        DOXYGEN_PROPERTY( QString version READ version )
        DOXYGEN_PROPERTY( int compatLevel READ compatLevel )
        DOXYGEN_PROPERTY( QString packagesXMLFileName READ packagesXMLFileName WRITE setPackagesXMLFileName )
        DOXYGEN_PROPERTY( QString updateSourcesXMLFileName READ updateSourcesXMLFileName WRITE setUpdateSourcesXMLFileName )
    public:
        Target();
        explicit Target( const QString& name );
        virtual ~Target();

        void setDirectory(const QString& dir);
        QString directory() const;

        QString name() const;
        QString version() const;
        int compatLevel() const;

        void setPackagesXMLFileName(const QString& fileName);
        QString packagesXMLFileName() const;
        PackagesInfo* packagesInfo() const;

        void addUpdateSource( const QString& name, const QString& title, 
                              const QString& description, const QUrl& url, int priority = -1 );

        void setUpdateSourcesXMLFileName(const QString& fileName);
        QString updateSourcesXMLFileName() const;
        UpdateSourcesInfo* updateSourcesInfo() const;

    private:
        friend class ::KDUpdater::Task;
        void addTask( Task * task );

    private:
        class Private;
        kdtools::pimpl_ptr< Private > d;
    };

}

#endif
