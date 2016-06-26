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

#include "updater.h"

#include <KDUpdater/Target>
#include <KDUpdater/PackagesInfo>
#include <KDUpdater/FileDownloaderFactory>
#include <KDUpdater/FileDownloader>
#include <KDUpdater/UpdateFinder>
#include <KDUpdater/Update>
#include <KDUpdater/UpdateSourcesInfo>
#include <KDUpdater/UpdateInstaller>

#include <QtCore/QFileInfo>
#include <QtCore/QFile>
#include <QtCore/QString>
#include <QtCore/QDir>
#include <QtCore/QVector>
#include <QtCore/QUrl>
#include <QtCore/QVariant>

#include <QtGui/QApplication>

/*!
   \class Updater updater.h
   \brief This class uses \ref KDUpdater to update this program

   The Updater is a simple class that can check if updates are present
   and install them on demand.
*/

class Updater::Private : public QObject
{
    Q_OBJECT
public:
    Private( Updater* parent = NULL );
    ~Private();
    void init();
    QString getInstalledAppVersion() const;
    QString getAppName() const;
    QString getUpdateVersion() const;
    bool checkForUpdates();
    bool hasUpdates() const;
    bool installUpdates();

private:
    Updater* m_parent;
    KDUpdater::Target m_updaterapp;
    bool m_valid;
    QString m_repository;
    QString m_updateVersion;
    bool m_searchedForUpdates;
    KDUpdater::FileDownloader* m_downloader;    
    KDUpdater::UpdateFinder* m_updateFinder;
    bool m_foundUpdates;
    QString m_componentName;
    QDate m_updateDate;

};

Updater::Private::Private( Updater* parent ) :
        m_parent(parent),
        m_valid( true ),
        m_searchedForUpdates( false ),
        m_downloader( NULL ),
        m_updateFinder ( new KDUpdater::UpdateFinder( &m_updaterapp ) ),
        m_foundUpdates( false )
{

}

Updater::Private::~Private()
{

}

bool Updater::Private::installUpdates()
{
    if ( !m_foundUpdates )
        return false;
    KDUpdater::UpdateInstaller inst( &m_updaterapp );
    inst.setUpdatesToInstall( m_updateFinder->updates() );
    inst.run();
    if ( inst.error() == 0 )
        m_updaterapp.packagesInfo()->updatePackage( m_componentName, getUpdateVersion(), m_updateDate );
    return true;
}

bool Updater::Private::hasUpdates() const
{
    return m_foundUpdates;
}

/*!
  checks if an update for an installed package is available
 */
bool Updater::Private::checkForUpdates()
{
    m_foundUpdates = false;
    m_updaterapp.addUpdateSource( getAppName(), getAppName(), QString(), QUrl::fromLocalFile( m_repository ) , 1);
    m_updaterapp.updateSourcesInfo()->setModified( false );
    m_updateFinder->setUpdateType( KDUpdater::PackageUpdate );
    m_updateFinder->run();
    Q_FOREACH( const KDUpdater::Update* info, m_updateFinder->updates() )
    {
        if ( m_updaterapp.packagesInfo()->findPackageInfo( info->data( QLatin1String( "Name" ) ).toString() ) > -1 )
        {
            m_updateVersion = info->data( QLatin1String( "Version" ) ).toString();
            m_componentName = info->data( QLatin1String( "Name" ) ).toString();
            m_updateDate = info->data( QLatin1String( "ReleaseDate" ) ).toDate();
            m_foundUpdates = true;
        }
    }
    if ( !m_foundUpdates )
    {
         m_updateVersion = QLatin1String( "No updates found" );
    }
    m_searchedForUpdates = true;
    return m_foundUpdates;
}

QString Updater::Private::getUpdateVersion() const
{
    if ( m_searchedForUpdates )
        return m_updateVersion;
    else
        return tr( "You have not searched for updates" );
}

/*!
  checks if the repository directory exists and creates the package.xml if not existent
 */
void Updater::Private::init()
{
    const QDir repodir( QLatin1String( REPO_DIR ) );
    m_repository = repodir.absolutePath();
    const QString appDirPath = QApplication::applicationDirPath();
    const QDir appDir( appDirPath );
#ifdef Q_OS_MAC
    const QFileInfo file ( appDir.absoluteFilePath( QLatin1String( "../../../packages.xml" ) ) );
#else
    const QFileInfo file ( appDir.absoluteFilePath( QLatin1String( "packages.xml" ) ) );
#endif
    if ( !file.exists() )
    {
        const bool success = QFile::copy( QLatin1String( ":/packages.xml" ), file.absoluteFilePath() );
        QFile permF( file.absoluteFilePath() );
        permF.setPermissions( QFile::ReadGroup | QFile::ReadOther | QFile::ReadOwner | QFile::WriteOwner );
        if ( !success )
        {
            m_valid = false;
            return;
        }
    }
    m_updaterapp.setPackagesXMLFileName( file.absoluteFilePath() );
}

bool Updater::checkForUpdates()
{
    return d->checkForUpdates();
}

bool Updater::installUpdates()
{
    return d->installUpdates();
}

QString Updater::getUpdateVersion() const
{
    return d->getUpdateVersion();
}

QString Updater::getAppName() const
{
    return d->getAppName();
}

QString Updater::Private::getAppName() const
{
    return m_updaterapp.name();
}

QString Updater::getInstalledAppVersion() const
{
    return d->getInstalledAppVersion();
}

QString Updater::Private::getInstalledAppVersion() const
{
    const KDUpdater::PackagesInfo* info = m_updaterapp.packagesInfo();
    const int appPackageIndex = info->findPackageInfo( QLatin1String( "com.kdab.kdupdaterdemo" ) );
    if ( appPackageIndex == -1)
        return QString();
    const KDUpdater::PackageInfo appInfo = info->packageInfos()[ appPackageIndex ];
    return appInfo.version;
}

bool Updater::hasUpdates() const
{
    return d->hasUpdates();
}

Updater::Updater() :
        d( new Private( this ) )
{
    d->init();
}

Updater::~Updater()
{
}

#include "updater.moc"
