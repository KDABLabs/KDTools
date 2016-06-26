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

#ifndef __KDTOOLS_KDUPDATERUPDATE_FINDER_H__
#define __KDTOOLS_KDUPDATERUPDATE_FINDER_H__

#include "kdupdater.h"
#include "kdupdatertask.h"
#include <KDToolsCore/pimpl_ptr.h>

#ifndef KDTOOLS_NO_COMPAT
# include "kdupdaterapplication.h"
#endif // KDTOOLS_NO_COMPAT

QT_BEGIN_NAMESPACE
class QUrl;

template< typename K, typename V >
class QMap;
QT_END_NAMESPACE

namespace KDUpdater
{
    class Target;
    class Update;
    struct UpdateSourceInfo;

    class KDTOOLS_UPDATER_EXPORT UpdateFinder : public Task
    {
        Q_OBJECT
        Q_PROPERTY( UpdateTypes updateType READ updateType WRITE setUpdateType )
        Q_PROPERTY( QString platformIdentifier READ platformIdentifier WRITE setPlatformIdentifier )

    public:
        explicit UpdateFinder( Target * target );
        ~UpdateFinder();

        Target * target() const;
        QList<Update*> updates() const;

        void setUpdateType( UpdateTypes type );
        UpdateTypes updateType() const;

        void setPlatformIdentifier( const QString & platformIdentifier );
        QString platformIdentifier() const;

#ifndef KDTOOLS_NO_COMPAT
        Application * application() const { return dynamic_cast<Application*>( target() ); }
#endif // KDTOOLS_NO_COMPAT

    private:
        void doRun();
        bool doStop();
        bool doPause();
        bool doResume();

        Update* constructUpdate( Target * target, const UpdateSourceInfo & sourceInfo,
                                 UpdateType type, const QUrl& updateUrl, const QMap< QString, QVariant >& data, quint64 compressedSize, quint64 uncompressedSize, const QByteArray& sha1sum );


    private:
        Q_PRIVATE_SLOT( d, void slotDownloadDone() )
        
        class Private;
        kdtools::pimpl_ptr< Private > d;
    };

}

#endif
