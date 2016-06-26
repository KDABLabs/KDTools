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

#include "kdupdaterupdateinstaller.h"
#include "kdupdaterpackagesinfo.h"
#include "kdupdatertarget.h"
#include "kdupdaterupdate.h"
#include "kdupdaterupdateoperationfactory.h"
#include "kdupdaterupdateoperation.h"
#include "kdupdaterufuncompressor_p.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>
#include <QDirIterator>
#include <QDomDocument>
#include <QDomElement>
#include <QDate>
#include <QStack>
#include <QVariant>

#include <memory>

using namespace KDUpdater;

static void removeDirectory(const QDir & dir)
{
    QDirIterator dirIt( dir.path(), QDir::Dirs| QDir::Files| QDir::NoDotAndDotDot );
    while( dirIt.hasNext() )
    {
        dirIt.next();
        const QFileInfo fi = dirIt.fileInfo();
        if( fi.isDir() )
        {
            const QDir childDir = fi.absoluteFilePath();
            removeDirectory( childDir );
            dir.rmdir( childDir.dirName() );
        }
        else if( fi.isFile() )
            QFile::remove( fi.absoluteFilePath() );
    }
}


/*!
 \internal
 */
class TempDirDeleter
{
public:
    explicit TempDirDeleter( const QString& path );
    explicit TempDirDeleter( const QStringList& paths = QStringList() );
    ~TempDirDeleter();
    void releaseAll();
    void release( const QString& path );

    void add( const QString& path );
    void add( const QDir& path );
    void add( const QStringList& paths );

private:
    QStringList paths;
};

TempDirDeleter::TempDirDeleter( const QString& path )
    : paths( QStringList() << path )
{
}

TempDirDeleter::TempDirDeleter( const QStringList& paths_ )
    : paths( paths_ )
{
}

TempDirDeleter::~TempDirDeleter()
{
    for( QStringList::const_iterator it = paths.constBegin(); it != paths.constEnd(); ++it )
    {
        const QString& path = *it;
        if( !path.isEmpty() )
            removeDirectory( QDir( path ) );
    }
}

void TempDirDeleter::releaseAll() {
    paths.clear();
}

void TempDirDeleter::release( const QString& path ) {
    paths.removeAll( path );
}

void TempDirDeleter::add( const QString& path )
{
    if( !paths.contains( path ) )
        paths.push_back( path );
}

void TempDirDeleter::add( const QDir& path )
{
    add( path.absolutePath() );
}

void TempDirDeleter::add( const QStringList& paths )
{
    for( QStringList::const_iterator it = paths.begin(); it != paths.end(); ++it )
        add( *it );
}

/*!
   \ingroup kdupdater
   \class KDUpdater::UpdateInstaller kdupdaterupdateinstaller.h KDUpdaterUpdateInstaller
   \brief Installs updates, given a list of \ref KDUpdater::Update objects

   This class installs updates, given a list of \ref KDUpdater::Update objects via the
   \ref setUpdatesToInstall() method and call \ref run(). To install the updates this class 
   performs the following for each update

   \li Downloads the update files from its source
   \li Unpacks update files into a temporary directory
   \li Parses and executes UpdateInstructions.xml by making use of \ref KDUpdater::UpdateOperation
   objects sourced via \ref KDUpdater::UpdateOperationFactory

   \note All temporary files created during the installation of the update will be destroyed
   immediately after the installation is complete.
*/
class UpdateInstaller::Private
{
public:
    explicit Private( UpdateInstaller* qq )
        : q( qq ),
          target( 0 ),
          updateDownloadDoneCount( 0 ),
          updateDownloadFailCount( 0 ),
          updateDownloadProgress( 0 ),
          totalUpdates( 0 ),
          tempDirDeleter( 0 ),
          canceled( false ),
          totalProgressPc( 0 ),
          currentProgressPc( 0 )
    {
    }

private:
    UpdateInstaller* const q;

public:
    Target* target;
    int updateDownloadDoneCount;
    int updateDownloadFailCount;
    int updateDownloadProgress;
    int totalUpdates;
    TempDirDeleter* tempDirDeleter;

    bool canceled;

    int totalProgressPc;
    int currentProgressPc;
    QList<Update*> updates;

    void resolveArguments(QStringList& args);

    void slotUpdateDownloadProgress();
    void slotUpdateDownloadDone();
    void slotUpdateDownloadFailed();
};

// next two are duplicated from kdupdaterupdatefinder.cpp:

static int computeProgressPercentage(int min, int max, int percent)
{
    return min + qint64(max-min) * percent / 100 ;
}

static int computePercent(int done, int total)
{
    return total ? done * Q_INT64_C(100) / total : 0 ;
}

/*!
   Constructs an instance of this class for the \ref KDUpdater::Target passed as
   parameter. Only updates meant for the specified target will be installed by this class.
*/
UpdateInstaller::UpdateInstaller(Target* target)
    : Task(QLatin1String( "UpdateInstaller" ), Stoppable ),
      d( new Private( this ) )
{
    d->target = target;
}

/*!
   Destructor
*/
UpdateInstaller::~UpdateInstaller()
{
}

/*!
   Returns the update target for which the update is being installed
*/
Target* UpdateInstaller::target() const
{
    return d->target;
}

/*!
   Use this function to let the installer know what updates are to be installed. The
   updates are actually installed when the \ref run() method is called on this class.
*/
void UpdateInstaller::setUpdatesToInstall(const QList<Update*>& updates)
{
    d->updates = updates;
}

/*!
   Returns the updates that would be installed when the next time the \ref run() method is called.
*/
QList<Update*> UpdateInstaller::updatesToInstall() const
{
    return d->updates;
}

/*!
   \internal
*/
void UpdateInstaller::doRun()
{
    d->tempDirDeleter = new TempDirDeleter;
    std::auto_ptr< TempDirDeleter > tempDirDeleter( d->tempDirDeleter );
    
    QList<Update*>& updates = d->updates;

    // First download all the updates
    d->updateDownloadDoneCount = 0;
    d->updateDownloadFailCount = 0;
    d->totalUpdates = updates.count();

    for( QList< Update* >::const_iterator it = updates.begin(); it != updates.end(); ++it )
    {
        if( d->canceled )
            return;

        Update* const update = *it;
        if( update->target() != d->target )
            continue;

        connect(update, SIGNAL(progressValue(int)), this, SLOT(slotUpdateDownloadProgress()));
        connect(update, SIGNAL(finished()), this, SLOT(slotUpdateDownloadDone()));
        connect(update, SIGNAL(error(int,QString)), this, SLOT(slotUpdateDownloadFailed()) );
        connect(update, SIGNAL(stopped()), this, SLOT(slotUpdateDownloadDone()));
        update->download();
    }

    d->totalProgressPc = updates.count() * 100;
    d->currentProgressPc = 0;

    // Wait until all updates have been downloaded
    while(d->updateDownloadDoneCount + d->updateDownloadFailCount != updates.count() && !d->canceled )
    {
        QCoreApplication::processEvents();

        // Normalized progress
        int progressPc = computePercent(d->currentProgressPc, d->totalProgressPc);

        // Bring the progress to within 50 percent
        progressPc = (progressPc>>1);

        // Report the progress
        reportProgress(progressPc, tr("Downloading updates..."));
    }
    
    // Global progress
    reportProgress(50, tr("Updates downloaded..."));

    // Save the current working directory of the application
    const QDir oldCWD = QDir::current();

    int pcDiff = computePercent(1, updates.count());
    pcDiff = computeProgressPercentage(50, 95, pcDiff) - 50;

    // Now install one update after another.
    int i = 0;
    for( QList< Update* >::const_iterator it = updates.begin(); it != updates.end(); ++it, ++i )
    {
        if( d->canceled )
            return;

        Update* const update = *it;

        // Global progress
        const QString msg = tr("Installing %1..").arg(update->name());
        const int minPc = pcDiff*i + 50;
        const int maxPc = minPc + pcDiff;
        reportProgress(minPc, msg);

        if( update->target() != d->target )
            continue;

        QDir::setCurrent(oldCWD.absolutePath());
        if (!installUpdate(update, minPc, maxPc)) {
            d->target->packagesInfo()->writeToDisk();
            return;
        }
    }

    d->target->packagesInfo()->writeToDisk();

    // Global progress
    reportProgress(95, tr("Finished installing updates. Now removing temporary files and directories.."));

    d->tempDirDeleter = 0;

    // Restore the current working directory of the application
    QDir::setCurrent(oldCWD.absolutePath());

    // Global progress
    reportProgress(100, tr("Removed temporary files and directories"));
    reportDone();
}

/*!
   \internal
*/
bool UpdateInstaller::doStop()
{
    for( QList< Update* >::const_iterator it = d->updates.begin(); it != d->updates.end(); ++it )
        (*it)->stop();
    d->canceled = true;
    return true;
}

/*!
   \internal
*/
bool UpdateInstaller::doPause()
{
    return false;
}

/*!
   \internal
*/
bool UpdateInstaller::doResume()
{
    return false;
}

bool UpdateInstaller::installUpdate(Update* update, int minPc, int maxPc)
{
    const QString updateName( update->name() );

    // Sanity checks
    if( !update->isDownloaded() )
    {
        QString msg = tr("Could not download update '%1'").arg( update->data( QString::fromLatin1( "Name" ) ).toString() );
        reportError(msg);
        return false;
    }

    // Step 1: Prepare a directory into which the UpdateFile will be unpacked.
    // If update file is C:/Users/PRASHA~1/AppData/Local/Temp/qt_temp.Hp1204 and
    // the target name "MyApplication"
    // Then the directory would be %USERDIR%/AppData/Local/Temp/MyApplication_Update1
    static int count = 0;
    const QString dirName = QString::fromLatin1("%1_Update%2").arg( d->target->name(), QString::number(count++) );
    const QString updateFile = update->downloadedFileName();
    const QFileInfo fi(updateFile);
    QDir dir( fi.absolutePath() );
    dir.mkdir( dirName );
    dir.cd( dirName );
    d->tempDirDeleter->add( dir );

    // Step 2: Unpack the update file into the update directory
    UFUncompressor uncompressor;
    uncompressor.setFileName( updateFile );
    uncompressor.setDestination( dir.absolutePath() );

    if (!uncompressor.uncompress()) {
        reportError(tr("Couldn't uncompress update: %1")
                    .arg(uncompressor.errorString()));
        return false;
    }

    // Step 3: Find out the directory in which UpdateInstructions.xml can be found
    QDir updateDir = dir;
    while( !updateDir.exists(QLatin1String( "UpdateInstructions.xml" )) )
    {
        const QString path = updateDir.absolutePath();
        const QFileInfoList fiList = updateDir.entryInfoList( QDir::Dirs |QDir::NoDotAndDotDot );
        if( fiList.isEmpty() )
        {
            QString msg = tr("Could not find UpdateInstructions.xml for %1").arg(update->name());
            reportError(msg);
            return false;
        }

        updateDir.cd(fiList.first().fileName());
    }

    // Set the application's current working directory as updateDir
    QDir::setCurrent(updateDir.absolutePath());

    // Step 4: Now load the UpdateInstructions.xml file
    QDomDocument doc;
    QFile file(updateDir.absoluteFilePath(QLatin1String( "UpdateInstructions.xml" )));
    if( !file.open(QFile::ReadOnly) )
    {
        const QString msg = tr("Could not read UpdateInstructions.xml of %1").arg(update->name());
        reportError(msg);
        return false;
    }
    if( !doc.setContent(&file) )
    {
        const QString msg = tr("Could not read UpdateInstructions.xml of %1").arg(update->name());
        reportError(msg);
        return false;
    }

    // Now parse and execute update operations
    const int operEListCount = doc.elementsByTagName( QLatin1String( "UpdateOperation" ) ).count();
    QString msg = tr("Installing %1").arg(updateName);

    QStack< UpdateOperation* > performedOperations;
    QStack< UpdateOperation* > updatesToDelete;
    performedOperations.reserve( operEListCount );
    updatesToDelete.reserve( operEListCount );

    const QDomElement firstOperation = doc.firstChildElement( QLatin1String( "UpdateInstructions" ) ).firstChildElement( QLatin1String( "UpdateOperation" ) );

    int i = 0;
    for( QDomElement operE = firstOperation; !operE.isNull(); operE = operE.nextSiblingElement( QLatin1String( "UpdateOperation" ) ), ++i )
    {
        int pc = computePercent( i + 1, operEListCount );
        pc = computeProgressPercentage(minPc, maxPc, pc);
        reportProgress(pc, msg);

        // Fetch the important XML elements in UpdateOperation
        const QDomElement nameE = operE.firstChildElement(QLatin1String( "Name" ));
        const QDomElement errorE = operE.firstChildElement(QLatin1String( "OnError" ));
        QDomElement argE = operE.firstChildElement(QLatin1String( "Arg" ));

        // Figure out information about the update operation to perform
        const QString operName = nameE.text();
        const QString onError = errorE.attribute(QLatin1String( "Action" ), QLatin1String( "Abort" ) );
        QStringList args;
        while( !argE.isNull() )
        {
            args << argE.text();
            argE = argE.nextSiblingElement(QLatin1String( "Arg" ));
        }

        //QString operSignature = QString::fromLatin1("%1(%2)").arg(operName, args.join( QLatin1String( ", ") ) );

        // Now resolve special fields in arguments
        d->resolveArguments(args);

        // Fetch update operation
        UpdateOperation* const updateOperation = UpdateOperationFactory::instance().create( operName, args, d->target );
        if( !updateOperation )
        {
            reportError( tr( "Update operation %1 not supported" ).arg( operName ) );

            if( onError == QLatin1String( "Continue" ) )
                continue;

            else if( onError == QLatin1String( "Abort" ) )
            {
                qDeleteAll( updatesToDelete );
                return false;
            }

            else if( onError == QLatin1String( "AskUser" ) )
            {
                // TODO:
                continue;
            }
        }

        // Now execute the update operation
        updateOperation->backup();
        updatesToDelete.push( updateOperation );
        if( !updateOperation->performOperation() )
        {
            reportError( tr("Cannot execute '%1'").arg( updateOperation->operationCommand() ) );
            
            updateOperation->undoOperation();

            if( onError == QLatin1String( "Continue" ) )
                continue;

            else if( onError == QLatin1String( "Abort" ) )
            {
                while( !performedOperations.isEmpty() )
                    performedOperations.pop()->undoOperation();
                qDeleteAll( updatesToDelete );
                return false;
            }

            else if( onError == QLatin1String( "AskUser" ) )
            {
                // TODO:
                continue;
            }
        }
        else
        {
            performedOperations.push( updateOperation );
        }
    }

    Q_FOREACH( UpdateOperation* updateOperation, update->operations() )
    {
        updateOperation->backup();
        if( !updateOperation->performOperation() )
        {
            reportError( tr("Cannot execute '%1'").arg( updateOperation->operationCommand() ) );
            
            updateOperation->undoOperation();
            
            while( !performedOperations.isEmpty() )
                performedOperations.pop()->undoOperation();
            return false;
        }
        else
        {
            performedOperations.push( updateOperation );
        }
    }

    // now delete all operations, this will remove the backups as well
    qDeleteAll( updatesToDelete );

    msg = tr("Finished installing update %1").arg(update->name());
    reportProgress(maxPc, msg);
    return true;
}

/*!
   \internal
*/
void UpdateInstaller::Private::slotUpdateDownloadProgress()
{
    // calculate the global download progress
    currentProgressPc = 0;
    
    for( QList< Update* >::const_iterator it = updates.begin(); it != updates.end(); ++it )
        currentProgressPc += (*it)->progressPercent();
}

/*!
   \internal
*/
void UpdateInstaller::Private::slotUpdateDownloadDone()
{
    ++updateDownloadDoneCount;
}

/*!
 * \internal
 */
void UpdateInstaller::Private::slotUpdateDownloadFailed()
{
    ++updateDownloadFailCount;
}

void UpdateInstaller::Private::resolveArguments(QStringList& args)
{
    for( QStringList::iterator it = args.begin(); it != args.end(); ++it )
    {
        QString arg = *it;

        arg = arg.replace(QLatin1String( "{APPDIR}" ), QDir::toNativeSeparators( target->directory() ) ); // backwards compat
        arg = arg.replace(QLatin1String( "{TARGETDIR}" ), QDir::toNativeSeparators( target->directory() ) );
        arg = arg.replace(QLatin1String( "{HOME}" ), QDir::toNativeSeparators( QDir::homePath() ) );
        arg = arg.replace(QLatin1String( "{APPNAME}" ), target->name());  // backwards compat
        arg = arg.replace(QLatin1String( "{TARGETNAME}" ), target->name());
        arg = arg.replace(QLatin1String( "{APPVERSION}" ), target->version());  // backwards compat
        arg = arg.replace(QLatin1String( "{TARGETVERSION}" ), target->version());
        arg = arg.replace(QLatin1String( "{CURPATH}" ), QDir::toNativeSeparators( QDir::currentPath() ) );
        arg = arg.replace(QLatin1String( "{ROOT}" ), QDir::toNativeSeparators( QDir::rootPath() ) );
        arg = arg.replace(QLatin1String( "{TEMP}" ), QDir::toNativeSeparators( QDir::tempPath() ) );

        *it = arg;
    }
}

#include "moc_kdupdaterupdateinstaller.cpp"
