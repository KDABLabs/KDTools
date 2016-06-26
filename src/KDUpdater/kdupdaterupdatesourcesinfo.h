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

#ifndef __KDTOOLS_KDUPDATERUPDATESOURCESINFO_H__
#define __KDTOOLS_KDUPDATERUPDATESOURCESINFO_H__

#include "kdupdater.h"
#include <KDToolsCore/pimpl_ptr.h>

#include <QtCore/QMetaType>
#include <QtCore/QObject>
#include <QtCore/QUrl>

#ifndef KDTOOLS_NO_COMPAT
# include "kdupdaterapplication.h"
#endif // KDTOOLS_NO_COMPAT

namespace KDUpdater
{
    class Target;

    struct KDTOOLS_UPDATER_EXPORT UpdateSourceInfo
    {
        UpdateSourceInfo();

        QString name;
        QString title;
        QString description;
        QUrl url;
        int priority;
    };

    KDTOOLS_UPDATER_EXPORT bool operator==( const UpdateSourceInfo & lhs, const UpdateSourceInfo & rhs );
    KDTOOLS_UPDATER_EXPORT bool operator<( const UpdateSourceInfo & lhs, const UpdateSourceInfo & rhs );
    KDTOOLS_MAKE_RELATION_OPERATORS( UpdateSourceInfo, inline )

    class KDTOOLS_UPDATER_EXPORT UpdateSourcesInfo : public QObject
    {
        Q_OBJECT
        Q_PROPERTY( Error error READ error )
        Q_PROPERTY( QString errorString READ errorString )
        Q_PROPERTY( QString fileName READ fileName WRITE setFileName )

    public:
        enum Error
        {
            NoError=0,
            NotYetReadError,
            CouldNotReadSourceFileError,
            InvalidXmlError,
            InvalidContentError,
            CouldNotSaveChangesError
        };
        
        Target * target() const;

        bool isValid() const;
        QString errorString() const;
        Error error() const;

        bool isModified() const;
        void setModified(bool modified);
        
        void setFileName(const QString& fileName);
        QString fileName() const;

        int updateSourceInfoCount() const;
        UpdateSourceInfo updateSourceInfo(int index) const;

        void addUpdateSourceInfo(const UpdateSourceInfo& info);
        void removeUpdateSourceInfo(const UpdateSourceInfo& info);
        void removeUpdateSourceInfoAt(int index);
        void setUpdateSourceInfoAt(int index, const UpdateSourceInfo& info);

#ifndef KDTOOLS_NO_COMPAT
        Application * application() const { return dynamic_cast<Application*>( target() ); }
#endif // KDTOOLS_NO_COMPAT

    protected:
        explicit UpdateSourcesInfo( Target * target );
        ~UpdateSourcesInfo();

    public Q_SLOTS:
        void refresh();

    Q_SIGNALS:
        void reset();
        void aboutToAddUpdateSourceInfo( int index, const UpdateSourceInfo& info );
        void updateSourceInfoAdded( const UpdateSourceInfo& info );
        
        void updateSourceInfoRemoved( const UpdateSourceInfo& info );
        void aboutToRemoveUpdateSourceInfo( const UpdateSourceInfo& info );

        void updateSourceInfoChanged( const UpdateSourceInfo& newInfo, const UpdateSourceInfo& oldInfo );

    private:
        friend class Target;
        class Private;
        kdtools::pimpl_ptr< Private > d;
    };
}

Q_DECLARE_METATYPE(KDUpdater::UpdateSourceInfo)

#endif
