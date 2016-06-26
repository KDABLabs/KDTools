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

#ifndef __KDTOOLS_KDUPDATERPACKAGESINFO_H__
#define __KDTOOLS_KDUPDATERPACKAGESINFO_H__

#include "kdupdater.h"
#include <KDToolsCore/pimpl_ptr.h>

#include <QtCore/QObject>
#include <QtCore/QDate>
#include <QtCore/QString>
#include <QtCore/QStringList>

#ifndef KDTOOLS_NO_COMPAT
# include "kdupdaterapplication.h"
#endif

namespace KDUpdater
{
    class Target;
    class UpdateInstaller;

    struct KDTOOLS_UPDATER_EXPORT PackageInfo
    {
        QString name;
        QString pixmap;
        QString title;
        QString description;
        QString version;
        QStringList dependencies;
        QDate lastUpdateDate;
        QDate installDate;
        quint64 uncompressedSize;
    };

    class KDTOOLS_UPDATER_EXPORT PackagesInfo : public QObject
    {
        Q_OBJECT
        Q_PROPERTY( QString targetName READ targetName WRITE setTargetName )
        Q_PROPERTY( QString targetVersion READ targetVersion WRITE setTargetVersion )
#ifndef KDTOOLS_NO_COMPAT
        Q_PROPERTY( QString applicationName READ applicationName WRITE setApplicationName )
        Q_PROPERTY( QString applicationVersion READ applicationVersion WRITE setApplicationVersion )
#endif
        Q_PROPERTY( int compatLevel READ compatLevel WRITE setCompatLevel )
        Q_PROPERTY( QString errorString READ errorString )
        Q_PROPERTY( QString fileName READ fileName WRITE setFileName )

    public:
        enum Error
        {
            NoError=0,
            NotYetReadError,
            CouldNotReadPackageFileError,
            InvalidXmlError,
            InvalidContentError
        };

        Target * target() const;

        bool isValid() const;
        QString errorString() const;
        Error error() const;
        
        void setFileName( const QString& fileName );
        QString fileName() const;

        void setTargetName( const QString & name );
        QString targetName() const;

        void setTargetVersion( const QString & version );
        QString targetVersion() const;

        int packageInfoCount() const;
        PackageInfo packageInfo( int index ) const;
        int findPackageInfo( const QString& pkgName ) const;
        QVector<KDUpdater::PackageInfo> packageInfos() const;

        int compatLevel() const;
        void setCompatLevel( int level );

        bool installPackage( const QString& pkgName, const QString& version, const QString& title = QString(), const QString& description = QString()
                           , const QStringList& dependencies = QStringList(), quint64 uncompressedSize = 0 );
        bool updatePackage( const QString &pkgName, const QString &version, const QDate &date );
        bool removePackage( const QString& pkgName );

#ifndef KDTOOLS_NO_COMPAT
        Application * application() const { return dynamic_cast<Application*>( target() ); }
        void setApplicationName( const QString & n ) { setTargetName( n ); }
        QString applicationName() const { return targetName(); }
        void setApplicationVersion( const QString & v ) { setTargetVersion( v ); }
        QString applicationVersion() const { return targetVersion(); }
#endif

    public Q_SLOTS:
        void refresh();

    Q_SIGNALS:
        void reset();

    protected:
        explicit PackagesInfo( Target * target=0 );
        ~PackagesInfo();

        void writeToDisk();

    private:
        friend class Target;
        friend class UpdateInstaller;
        class Private;
        kdtools::pimpl_ptr< Private > d;
    };

}

#endif
