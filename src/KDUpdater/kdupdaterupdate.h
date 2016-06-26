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

#ifndef __KDTOOLS_KDUPDATERUPDATE_H__
#define __KDTOOLS_KDUPDATERUPDATE_H__

#include "kdupdater.h"
#include "kdupdatertask.h"
#include <KDToolsCore/pimpl_ptr.h>

#ifndef KDTOOLS_NO_COMPAT
# include "kdupdaterapplication.h"
#endif

QT_BEGIN_NAMESPACE
template< typename K, typename V >
class QMap;

template< typename T >
class QList;

class QDate;
class QUrl;
QT_END_NAMESPACE

namespace KDUpdater
{
    class Target;
    struct UpdateSourceInfo;
    class UpdateFinder;
    class UpdateOperation;

    class KDTOOLS_UPDATER_EXPORT Update : public Task
    {
        Q_OBJECT
        Q_PROPERTY( bool canDownload READ canDownload )
        Q_PROPERTY( quint64 compressedSize READ compressedSize )
        Q_PROPERTY( bool downloaded READ isDownloaded )
        Q_PROPERTY( QString downloadedFileName READ downloadedFileName )
        Q_PROPERTY( QDate releaseDate READ releaseDate )
        Q_PROPERTY( UpdateType type READ type )
        Q_PROPERTY( quint64 uncompressedSize READ uncompressedSize )
        Q_PROPERTY( QUrl updateUrl READ updateUrl )

    public:
        ~Update();

        Target * target() const;

        UpdateType type() const;
        QUrl updateUrl() const;
        QDate releaseDate() const;
        QVariant data( const QString& name ) const;
        UpdateSourceInfo sourceInfo() const;

        bool canDownload() const;
        bool isDownloaded() const;
        void download() { run(); }
        QString downloadedFileName() const;

        QList<UpdateOperation*> operations() const;

        quint64 compressedSize() const;
        quint64 uncompressedSize() const;

#ifndef KDTOOLS_NO_COMPAT
        Application * application() const { return dynamic_cast<Application*>( target() ); }
#endif // KDTOOLS_NO_COMPAT

    private:
        Q_PRIVATE_SLOT( d, void downloadProgress( int ) )
        Q_PRIVATE_SLOT( d, void downloadAborted( const QString& msg ) )
        Q_PRIVATE_SLOT( d, void downloadCompleted() )

        friend class UpdateFinder;
        class Private;
        kdtools::pimpl_ptr< Private > d;

        void doRun();
        bool doStop();
        bool doPause();
        bool doResume();

        Update( Target* target, const UpdateSourceInfo& sourceInfo,
               UpdateType type, const QUrl& updateUrl, const QMap<QString, QVariant>& data, quint64 compressedSize, quint64 uncompressedSize, const QByteArray& sha1sum );
    };
}

#endif
