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

#include "kdupdaterupdatefinder.h"
#include "kdupdatertarget.h"
#include "kdupdaterupdatesourcesinfo.h"
#include "kdupdaterpackagesinfo.h"
#include "kdupdaterupdate.h"
#include "kdupdaterfiledownloader_p.h"
#include "kdupdaterfiledownloaderfactory.h"
#include "kdupdaterupdatesinfo_p.h"

#include <QCoreApplication>
#include <QDebug>

/*!
   \ingroup kdupdater
   \class KDUpdater::UpdateFinder kdupdaterupdatefinder KDUpdaterUpdateFinder
   \brief Finds updates applicable for a \ref KDUpdater::Target

   The KDUpdater::UpdateFinder class helps in searching for updates and installing it on the target. The
   class basically processes the target's \ref KDUpdater::PackagesInfo and the UpdateXMLs it aggregates
   from all the update sources described in KDUpdater::UpdateSourcesInfo and populates a list of
   \ref KDUpdater::Update objects. This list can then be passed to \ref KDUpdater::UpdateInstaller for
   actually downloading and installing the updates.


   Usage:
   \code
   KDUpdater::UpdateFinder updateFinder( target );
   QProgressDialog finderProgressDlg;

   QObject::connect( &updateFinder, SIGNAL(progressValue(int)),
   &finderProgressDlg, SLOT(setValue(int)));
   QObject::connect( &updateFinder, SIGNAL(computeUpdatesCompleted()),
   &finderProgressDlg, SLOT(accept()));
   QObject::connect( &updateFinder, SIGNAL(computeUpdatesCanceled()),
   &finderProgressDlg, SLOT(reject()));

   QObject::connect( &finderProgressDlg, SIGNAL(canceled()),
   &updateFinder, SLOT(cancelComputeUpdates()));

   updateFinder.run();
   finderProgressDlg.exec();

// Control comes here after update finding is done or canceled.

QList<KDUpdater::Update*> updates = updateFinder.updates();
KDUpdater::UpdateInstaller updateInstaller;
updateInstaller.installUpdates( updates );

\endcode
*/

static const int DownloadPercentageBegin = 0;
static const int DownloadPercentageEnd = 50;

static const int ComputeUpdatesPercentageBegin = 50;
static const int ComputeUpdatesPercentageEnd = 100;


static QString suggest_platform_identifier() {
#if   defined(Q_OS_WIN)
    return QLatin1String("Windows");
#elif defined(Q_OS_LINUX)
    return QLatin1String("Linux");
#elif defined(Q_WS_MAC)
    return QLatin1String("MacOSX");
#else
    return QString();
#endif
}

using namespace KDUpdater;

//
// Private
//
class UpdateFinder::Private
{
public:
    Private( UpdateFinder* qq ) :
        q( qq ),
        target( 0 ),
        updateType(PackageUpdate),
        platformIdentifier( suggest_platform_identifier() )
    {}

    ~Private()
    {
        qDeleteAll( updates );
        qDeleteAll( updatesInfoList );
        qDeleteAll( updateXmlFDList );
    }
    
    UpdateFinder* q;
    Target * target;
    QList<Update*> updates;
    UpdateTypes updateType;
    QString platformIdentifier;

    // Temporary structure that notes down information about updates.
    bool cancel;
    int downloadCompleteCount;
    QVector<UpdateSourceInfo> updateSourceInfoList;
    QList<UpdatesInfo*> updatesInfoList;
    QList<FileDownloader*> updateXmlFDList;

    void clear();
    void computeUpdates();
    void cancelComputeUpdates();
    bool downloadUpdateXMLFiles();
    bool computeApplicableUpdates();

    QVector<UpdateInfo> applicableUpdates( const UpdatesInfo* updatesInfo, bool addNewPackages=false );
    void createUpdateObjects(const UpdateSourceInfo& sourceInfo,
                             const QVector<UpdateInfo>& updateInfoList);
    bool checkForUpdatePriority(const UpdateSourceInfo& sourceInfo,
                                const UpdateInfo& updateInfo);
    int pickUpdateFileInfo(const QVector<UpdateFileInfo>& updateFiles);
    void slotDownloadDone();
};


static int computeProgressPercentage(int min, int max, int percent)
{
    return min + qint64(max-min) * percent / 100;
}

static int computePercent(int done, int total)
{
    return total ? done * Q_INT64_C(100) / total : 0 ;
}

/*!
   \internal

   Releases all internal resources consumed while downloading and computing updates.
*/
void UpdateFinder::Private::clear()
{
    qDeleteAll( updates );
    updates.clear();
    qDeleteAll( updatesInfoList );
    updatesInfoList.clear();
    qDeleteAll( updateXmlFDList );
    updateXmlFDList.clear();
    updateSourceInfoList.clear();

    downloadCompleteCount = 0;
}

/*!
   \internal

   This method computes the updates that can be applied on the target by
   studying the target's \ref KDUpdater::PackagesInfo object and the UpdateXML files
   from each of the update sources described in \ref KDUpdater::UpdateSourcesInfo.

   This function can take a long time to complete. The following signals are emitted
   during the execution of this function

   The function creates \ref KDUpdater::Update objects on the stack. All KDUpdater::Update objects
   are made children of the target associated with this finder.

   The update sources are fetched from the \ref KDUpdater::UpdateSourcesInfo object associated with
   the target. Package information is extracted from the \ref KDUpdater::PackagesInfo object
   associated with the target.

   \note Each time this function is called, all the previously computed updates are discarded
and its resources are freed.
*/
void UpdateFinder::Private::computeUpdates()
{
    // Computing updates is done in two stages
    // 1. Downloading Update XML files from all the update sources
    // 2. Matching updates with Package XML and figuring out available updates

    cancel = false;
    clear();

    // First do some quick sanity checks on the packages info
    PackagesInfo * packages = target->packagesInfo();
    if( !packages ) {
        q->reportError(tr("Could not access the package information of this target"));
        return;
    }
    if( !packages->isValid() ) {
        q->reportError(packages->errorString());
        return;
    }

    // Now do some quick sanity checks on the update sources info
    UpdateSourcesInfo * sources = target->updateSourcesInfo();
    if( !sources ) {
        q->reportError(tr("Could not access the update sources information of this target"));
        return;
    }
    if( !sources->isValid() ) {
        q->reportError(sources->errorString());
        return;
    }

    // Now we can start...

    // Step 1: 0 - 49 percent
    if(!downloadUpdateXMLFiles() || cancel)
    {
        clear();
        return;
    }

    // Step 2: 50 - 100 percent
    if(!computeApplicableUpdates() || cancel)
    {
        clear();
        return;
    }

    // All done
    q->reportProgress( 100, tr("%1 updates found").arg(updates.count()) );
    q->reportDone();
}

/*!
   \internal

   Cancels the computation of updates.

   \sa \ref computeUpdates()
*/
void UpdateFinder::Private::cancelComputeUpdates()
{
    cancel = true;
}

/*!
   \internal

   This function downloads Updates.xml from all the update sources. A single target can potentially
   have several update sources, hence we need to be asynchronous in downloading updates from different
   sources.

   The function basically does this for each update source
   a) Create a KDUpdater::FileDownloader and KDUpdater::UpdatesInfo for each update
   b) Triggers the download of Updates.xml from each file downloader.
   c) The downloadCompleted(), downloadCanceled() and downloadAborted() signals are connected
   in each of the downloaders. Once all the downloads are complete and/or aborted, the next stage
   would be done.

   The function gets into an event loop until all the downloads are complete.
*/
bool UpdateFinder::Private::downloadUpdateXMLFiles()
{
    if( !target )
        return false;

    const UpdateSourcesInfo * const updateSources = target->updateSourcesInfo();
    if( !updateSources )
        return false;

    // Create KDUpdater::FileDownloader and KDUpdater::UpdatesInfo for each update
    for(int i=0; i<updateSources->updateSourceInfoCount(); i++)
    {
        const UpdateSourceInfo info = updateSources->updateSourceInfo(i);
        const QUrl updateXmlUrl = QString::fromLatin1("%1/Updates.xml").arg(info.url.toString());
        KDUpdater::FileDownloader* downloader = FileDownloaderFactory::instance().create(updateXmlUrl.scheme(),  q);
        if( !downloader )
            continue;

        downloader->setUrl(updateXmlUrl);
        downloader->setAutoRemoveDownloadedFile(true);

        UpdatesInfo* const updatesInfo = new UpdatesInfo;
        updateSourceInfoList.append(info);
        updateXmlFDList.append(downloader);
        updatesInfoList.append(updatesInfo);

        connect(downloader, SIGNAL(downloadCompleted()),
                q, SLOT(slotDownloadDone()));
        connect(downloader, SIGNAL(downloadCanceled()),
                q, SLOT(slotDownloadDone()));
        connect(downloader, SIGNAL(downloadAborted(QString)),
                q, SLOT(slotDownloadDone()));
    }

    // Trigger download of Updates.xml file
    downloadCompleteCount = 0;
    for( QList< FileDownloader* >::const_iterator it = updateXmlFDList.begin(); it != updateXmlFDList.end(); ++it )
        (*it)->download();

    // Wait until all downloaders have completed their downloads.
    while(1)
    {
        QCoreApplication::processEvents();
        if( cancel )
            return false;
        if( downloadCompleteCount == updateXmlFDList.count())
            break;

        const int pc = computePercent(downloadCompleteCount, updateXmlFDList.count());
        q->reportProgress(pc, tr("Downloading Updates.xml from update-sources"));
    }

    // All the downloaders have now either downloaded or aborted the
    // donwload of update XML files.

    // Lets now get rid of update sources whose Updates.xml could not be downloaded
    for(int i=0; i<updateXmlFDList.count(); i++)
    {
        const FileDownloader* const downloader = updateXmlFDList[i];
        if( downloader->isDownloaded() )
            continue;

        const UpdateSourceInfo& info = updateSourceInfoList[i];
        const QString msg = tr("Could not download updates from %1 ('%2')").arg(info.name, info.url.toString());
        q->reportError(msg);

        delete updatesInfoList[i];
        delete downloader;
        updateXmlFDList.removeAt(i);
        updatesInfoList.removeAt(i);
        updateSourceInfoList.remove( i );
        --i;
    }

    if (updatesInfoList.isEmpty()) {
        return false;
    }

    // Lets parse the downloaded update XML files and get rid of the downloaders.
    for(int i=0; i<updateXmlFDList.count(); i++)
    {
        const FileDownloader* const downloader = updateXmlFDList[i];
        UpdatesInfo* const updatesInfo = updatesInfoList[i];

        updatesInfo->setFileName( downloader->downloadedFileName() );

        if (!updatesInfo->isValid()) {
            QString msg = updatesInfo->errorString();
            q->reportError(msg);

            delete updatesInfoList[i];
            delete downloader;
            updateXmlFDList.removeAt(i);
            updatesInfoList.removeAt(i);
            --i;
        }
    }
    qDeleteAll( updateXmlFDList );
    updateXmlFDList.clear();

    if (updatesInfoList.isEmpty()) {
        return false;
    }

    q->reportProgress( 49, tr("Updates.xml file(s) downloaded from update sources") );
    return true;
}

/*!
   \internal

   This function runs through all the KDUpdater::UpdatesInfo objects created during
   the downloadUpdateXMLFiles() method and compares it with the data contained in
   KDUpdater::PackagesInfo. There by figures out whether an update is applicable for
   this target or not.
*/
bool UpdateFinder::Private::computeApplicableUpdates()
{
    if( updateType & CompatUpdate )
    {
        UpdateInfo compatUpdateInfo;
        UpdateSourceInfo compatUpdateSourceInfo;

        // Required compat level
        const int reqCompatLevel = target->compatLevel() + 1 ;

        q->reportProgress(60, tr("Looking for compatibility update..."));

        // We are only interested in compat updates.
        for(int i=0; i<updatesInfoList.count(); i++)
        {
            const UpdatesInfo* const info = updatesInfoList[i];
            UpdateSourceInfo updateSource = updateSourceInfoList[i];

            // If we already have a compat update, just check if the source currently being
            // considered has a higher priority or not.
            if(compatUpdateInfo.data.contains( QLatin1String( "CompatLevel" ) ) && updateSource.priority < compatUpdateSourceInfo.priority)
                continue;

            // Lets look for comapt updates that provide compat level one-higher than
            // the target's current compat level.
            const QVector<UpdateInfo> updatesInfo = info->updatesInfo( CompatUpdate, reqCompatLevel );

            if( updatesInfo.count() == 0 )
                continue;

            compatUpdateInfo = updatesInfo.at( 0 );
            compatUpdateSourceInfo = updateSource;
        }

        const bool found = (compatUpdateInfo.data.contains( QLatin1String( "CompatLevel" ) ));
        if(found)
        {
            q->reportProgress(80, tr("Found compatibility update.."));

            // Lets create an update for this compat update.
            const QString updateName = tr("Compatibility level %1 update").arg(reqCompatLevel);

            // Pick a update file based on arch and OS.
            const int pickUpdateFileIndex = pickUpdateFileInfo(compatUpdateInfo.updateFiles);
            if(pickUpdateFileIndex < 0)
            {
                q->reportError(tr("Compatibility update for the required architecture and hardware configuration was not found"));
                q->reportProgress(100, tr("Compatibility update not found"));
                return false;
            }

            const UpdateFileInfo& fileInfo = compatUpdateInfo.updateFiles.at( pickUpdateFileIndex );

            // Create an update for this entry
            const QUrl url = QString::fromLatin1( "%1/%2" ).arg( compatUpdateSourceInfo.url.toString(), fileInfo.fileName );
            Update * const  update = q->constructUpdate(target,
                                                              compatUpdateSourceInfo,
                                                              CompatUpdate, url,
                                                              compatUpdateInfo.data, fileInfo.compressedSize, fileInfo.uncompressedSize, fileInfo.sha1sum );

            // Register the update
            updates.append(update);

            // Done
            q->reportProgress(100, tr("Compatibility update found"));
        }
        else
            q->reportProgress(100, tr("No compatibility updates found"));
    }
    else if ( updateType & PackageUpdate )
    {
        // We are not looking for normal updates, not compat ones.
        int i = 0;
        for( QList< UpdatesInfo* >::const_iterator it = updatesInfoList.begin(); it != updatesInfoList.end(); ++it, ++i )
        {
            // Fetch updates applicable to this target.
            const UpdatesInfo* const info = *it;
            const QVector<UpdateInfo> updates = applicableUpdates(info, updateType & NewPackage);
            if( updates.isEmpty() )
                continue;

            if( cancel )
                return false;
            const UpdateSourceInfo& updateSource = updateSourceInfoList[i];

            // Create KDUpdater::Update objects for updates that have a valid
            // UpdateFile
            createUpdateObjects(updateSource, updates);
            if( cancel )
                return false;

            // Report progress
            int pc = computePercent(i, updatesInfoList.count());
            pc = computeProgressPercentage( ComputeUpdatesPercentageBegin, ComputeUpdatesPercentageEnd, pc );  // percentage from 50% to 100% is for the calculate updates stuff
            q->reportProgress( pc, tr("Computing applicable updates") );
        }
    }

    q->reportProgress( 99, tr("Application updates computed") );
    return true;
}

QVector<UpdateInfo> UpdateFinder::Private::applicableUpdates( const UpdatesInfo* updatesInfo, bool addNewPackages)
{
    QVector<UpdateInfo> retList;

    if( !updatesInfo || updatesInfo->updateInfoCount( PackageUpdate ) == 0 )
        return retList;

    const PackagesInfo * const packages = this->target->packagesInfo();
    if( !packages )
        return retList;

    // Check to see if the updates info contains updates for any target
    const bool anyApp
        =  updatesInfo->targetName() == QLatin1String( "{AnyTarget}" )
        || updatesInfo->targetName() == QLatin1String( "{AnyApplication}" ) ; // backwards compat
    int appNameIndex = -1;

    if( !anyApp )
    {
        // updatesInfo->targetName() describes one target or a series of
        // target names separated by commas.
        QString appName = updatesInfo->targetName();
        appName = appName.replace(QLatin1String( ", " ),
                                  QLatin1String( "," ));
        appName = appName.replace(QLatin1String( " ," ),
                                  QLatin1String( "," ));

        // Catch hold of app names contained updatesInfo->targetName()
        const QStringList apps = appName.split(QLatin1String( "," ), QString::SkipEmptyParts);
        appNameIndex = apps.indexOf( this->target->name() );

        // If the target appName isnt one of the app names, then
        // the updates are not applicable.
        if( appNameIndex < 0 )
            return retList;
    }

    // Check to see if the update repository versions match with app version
    if( !anyApp )
    {
        QString appVersion = updatesInfo->targetVersion();
        appVersion = appVersion.replace(QLatin1String( ", " ), QLatin1String( "," ));
        appVersion = appVersion.replace(QLatin1String( " ," ), QLatin1String( "," ));
        const QStringList versions = appVersion.split(QLatin1String( "," ), QString::SkipEmptyParts);

        if( appNameIndex >= versions.count() )
            return retList; // please give us well formatted Updates.xml files.

        const QString version = versions[appNameIndex];
        if( compareVersion( this->target->version(), version ) != 0 )
            return retList;
    }

    // Check to see if version numbers match. This means that the version
    // number of the update should be greater than the version number of
    // the package that is currently installed.
    const QVector<UpdateInfo> updateList = updatesInfo->updatesInfo( PackageUpdate );
    for( QVector< UpdateInfo >::const_iterator it = updateList.begin(); it != updateList.end(); ++it )
    {
        const UpdateInfo& updateInfo = *it;
        if( !addNewPackages )
        {
            int pkgInfoIdx = packages->findPackageInfo( updateInfo.data.value( QLatin1String( "Name" ) ).toString() );
            if( pkgInfoIdx < 0 )
                continue;

            const PackageInfo pkgInfo = packages->packageInfo( pkgInfoIdx );
            // First check to see if the update version is more than package version
            const QString updateVersion = updateInfo.data.value( QLatin1String( "Version" ) ).toString();
            const QString& pkgVersion = pkgInfo.version;
            if( KDUpdater::compareVersion(updateVersion, pkgVersion) <= 0 )
                continue;

            // It is quite possible that we may have already installed the update.
            // Lets check the last update date of the package and the release date
            // of the update. This way we can compare and figure out if the update
            // has been installed or not.
            const QDate& pkgDate = pkgInfo.lastUpdateDate;
            const QDate updateDate = updateInfo.data.value( QLatin1String( "ReleaseDate" ) ).toDate();
            if( pkgDate > updateDate )
                continue;
        }
        // Bingo!, we found an update :-)
        retList.append(updateInfo);
    }

    return retList;
}

void UpdateFinder::Private::createUpdateObjects(const UpdateSourceInfo& sourceInfo, const QVector<UpdateInfo>& updateInfoList)
{
    for( QVector< UpdateInfo >::const_iterator it = updateInfoList.begin(); it != updateInfoList.end(); ++it )
    {
        const UpdateInfo& info = *it;
        // Compat level checks
        if( info.data.contains( QLatin1String( "RequiredCompatLevel" ) ) &&
            info.data.value( QLatin1String( "RequiredCompatLevel" ) ).toInt() != target->compatLevel() )
        {
            qDebug() << "Update \"" << info.data.value( QLatin1String( "Name" ) ).toString() << "\" at \""
                     << sourceInfo.name << "\"(\"" << sourceInfo.url.toString() << "\") requires a different compat level";
            continue; // Compatibility level mismatch
        }

        // If another update of the same name exists, then use the update coming from
        // a higher priority.
        if( !checkForUpdatePriority(sourceInfo, info) )
        {
            qDebug() << "Skipping Update \""
                     << info.data.value( QLatin1String( "Name" ) ).toString()
                     << "\" from \""
                     << sourceInfo.name
                     << "\"(\""
                     << sourceInfo.url.toString()
                     << "\") because an update with the same name was found from a higher priority location";

            continue;
        }

        // Pick a update file based on arch and OS.
        const int pickUpdateFileIndex = this->pickUpdateFileInfo(info.updateFiles);
        if(pickUpdateFileIndex < 0)
            continue;

        const UpdateFileInfo& fileInfo = info.updateFiles.at( pickUpdateFileIndex );

        // Create an update for this entry
        const QUrl url( QString::fromLatin1("%1/%2").arg( sourceInfo.url.toString(), fileInfo.fileName ) );
        Update * const update = q->constructUpdate( target, sourceInfo, PackageUpdate, url, info.data, fileInfo.compressedSize, fileInfo.uncompressedSize, fileInfo.sha1sum );

        // Register the update
        this->updates.append(update);
    }
}

bool UpdateFinder::Private::checkForUpdatePriority(const UpdateSourceInfo& sourceInfo, const UpdateInfo& updateInfo)
{
    for( QList< Update* >::iterator it = updates.begin(); it != updates.end(); ++it )
    {
        Update* const update = *it;
        if( update->data( QLatin1String( "Name" ) ).toString() != updateInfo.data.value( QLatin1String( "Name" ) ).toString() )
            continue;

        // Bingo, update was previously found elsewhere.

        // If the existing update comes from a higher priority server, then cool :)
        if( update->sourceInfo().priority > sourceInfo.priority )
            return false;

        // If the existing update has a higher version number, keep it
        if ( compareVersion(update->data( QLatin1String( "Version" ) ).toString(),
                                       updateInfo.data.value( QLatin1String( "Version" ) ).toString()) > 0)
            return false;

        // Otherwise the old update must be deleted.
        updates.erase( it );
        delete update;

        return true;
    }

    // No update by that name was found, so what we have is a priority update.
    return true;
}

int UpdateFinder::Private::pickUpdateFileInfo(const QVector<UpdateFileInfo>& updateFiles)
    {
    int i = 0;
    Q_FOREACH( const UpdateFileInfo & ufi, updateFiles )
        if ( QRegExp( ufi.platformRegEx ).exactMatch( platformIdentifier ) )
        return i;
        else
            ++i;
    return -1;
}



//
// UpdateFinder
//

/*!
   Constructs a update finder for a given \ref KDUpdater::Target.
*/
UpdateFinder::UpdateFinder( Target * target )
    : Task( QLatin1String( "UpdateFinder" ), Stoppable ),
      d( new Private( this ) )
{
    d->target = target;
}

/*!
   Destructor
*/
UpdateFinder::~UpdateFinder()
{
}

/*!
   Returns a pointer to the update target for which this function computes all
   the updates.
*/
Target* UpdateFinder::target() const
{
    return d->target;
}

/*!
   Returns a list of KDUpdater::Update objects. The update objects returned in this list
   are made children of the \ref KDUpdater::Target object associated with this class.
*/
QList<Update*> UpdateFinder::updates() const
{
    return d->updates;
}

void UpdateFinder::setUpdateType(UpdateTypes type)
{
    d->updateType = type;
}

/*!
  \property KDUpdater::UpdateFinder::updateType

   Specifies the type of updates searched. By default, UpdateFinder is only looking for
   Package Updates.

   Get this property's value using %updateType(), and set it
   using %setUpdateType().
*/
UpdateTypes UpdateFinder::updateType() const
{
    return d->updateType;
}

void UpdateFinder::setPlatformIdentifier( const QString & platformIdentifier )
{
    d->platformIdentifier = platformIdentifier;
}

/*!
  \property KDUpdater::UpdateFinder::platformIdentifier

  Specifies the identifier for the platform we want to find updates for.

  The defaults are:
  <table>
  <tr><th>Platform</th><th>Identifier</th></tr>
  <tr><td>Windows</td><td>Windows</td></tr>
  <tr><td>Linux</td><td>Linux</td></tr>
  <tr><td>Mac OS X</td><td>MacOSX</td></tr>
  <tr><td>other</td><td><em>none</em></td></tr>
  </table>

  However, we strongly suggest you define your own platform
  identification scheme and not rely on the defaults.

  Get this property's value using %platformIdentifier(), and set it 
  using %setPlatformIdentifier().
*/

QString UpdateFinder::platformIdentifier() const
{
    return d->platformIdentifier;
}

/*!
   \internal

   Implemented from \ref KDUpdater::Task::doStart().
*/
void UpdateFinder::doRun()
{
    d->computeUpdates();
}

/*!
   \internal

   Implemented form \ref KDUpdater::Task::doStop()
*/
bool UpdateFinder::doStop()
{
    d->cancelComputeUpdates();

    // Wait until the cancel has actually happened, and then return.
    // Thinking of using QMutex for this. Frank/Till any suggestions?

    return true;
}

/*!
   \internal

   Implemented form \ref KDUpdater::Task::doStop()
*/
bool UpdateFinder::doPause()
{
    // Not a pausable task
    return false;
}

/*!
   \internal

   Implemented form \ref KDUpdater::Task::doStop()
*/
bool UpdateFinder::doResume()
{
    // Not a pausable task, hence it is not resumable as well
    return false;
}

/*!
   \internal
*/
void UpdateFinder::Private::slotDownloadDone()
{
    ++downloadCompleteCount;

    int pc = computePercent(downloadCompleteCount, updateXmlFDList.count());
    pc = computeProgressPercentage( DownloadPercentageBegin, DownloadPercentageEnd, pc ); // percentage 0% to 50% is for the downloads
    q->reportProgress( pc, tr("Downloading Updates.xml from update sources") );
}

/*!
   \internal
 */
Update* UpdateFinder::constructUpdate( Target* target, const UpdateSourceInfo& sourceInfo,
                                                             UpdateType type, const QUrl& updateUrl, const QMap< QString, QVariant >& data, quint64 compressedSize, quint64 uncompressedSize, const QByteArray& sha1sum )
{
    return new Update( target, sourceInfo, type, updateUrl, data, compressedSize, uncompressedSize, sha1sum );
}


/*!
   \ingroup kdupdater

   This function compares two version strings \c v1 and \c v2 and returns
   -1, 0 or +1 based on the following rule

   \li Returns 0 if v1 == v2
   \li Returns -1 if v1 < v2
   \li Returns +1 if v1 > v2

   The function is very similar to \c strcmp(), except that it works on version strings.

   Example:
   \code

   KDUpdater::compareVersion("2.0", "2.1"); // Returns -1
   KDUpdater::compareVersion("2.1", "2.0"); // Returns +1
   KDUpdater::compareVersion("2.0", "2.0"); // Returns 0
   KDUpdater::compareVersion("2.1", "2.1"); // Returns 0

   KDUpdater::compareVersion("2.0", "2.x"); // Returns 0
   KDUpdater::compareVersion("2.x", "2.0"); // Returns 0

   KDUpdater::compareVersion("2.0.12.4", "2.1.10.4"); // Returns -1
   KDUpdater::compareVersion("2.0.12.x", "2.0.x");    // Returns 0
   KDUpdater::compareVersion("2.1.12.x", "2.0.x");    // Returns +1
   KDUpdater::compareVersion("2.1.12.x", "2.x");      // Returns 0
   KDUpdater::compareVersion("2.x", "2.1.12.x");      // Returns 0

   \endcode
*/
int KDUpdater::compareVersion(const QString& v1, const QString& v2)
{
    // For tests refer VersionCompareFnTest testcase.

    // Check for equality
    if( v1 == v2 )
        return 0;

    // Split version numbers across .
    const QStringList v1_comps = v1.split( QLatin1Char( '.') );
    const QStringList v2_comps = v2.split( QLatin1Char( '.') );

    // Check each component of the version
    int index = 0;
    while(1)
    {
        if( index >= v1_comps.count() || index >= v2_comps.count() )
            break;

        bool v1_ok, v2_ok;
        int v1_comp = v1_comps[index].toInt(&v1_ok);
        int v2_comp = v2_comps[index].toInt(&v2_ok);

        if(!v1_ok)
        {
            if(v1_comps[index] == QLatin1String( "x" ) )
                return 0;
        }
        else if(!v2_ok)
        {
            if(v2_comps[index] == QLatin1String( "x") )
                return 0;
        }
        if( !v1_ok && !v2_ok )
        {
            return v1_comps[ index ].compare( v2_comps[ index ] );
        }

        if( v1_comp < v2_comp )
            return -1;

        if( v1_comp > v2_comp )
            return +1;

        // v1_comp == v2_comp
        ++index;
    }

    if( index < v2_comps.count() )
        return +1;

    if( index < v1_comps.count() )
        return -1;

    // Controversial return. I hope this never happens.
    return 0;
}

/*!
   \ingroup kdupdater
   
   Compares the version strings \a v1 and \a v2 for \a v1 being less 
   than \a v2. Returns true in that case. Otherwise returns false.
*/
bool KDUpdater::versionLessThan( const QString& v1, const QString& v2 )
{
    return compareVersion( v1, v2 ) < 0;
}

#include "moc_kdupdaterupdatefinder.cpp"
