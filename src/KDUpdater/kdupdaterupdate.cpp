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

#include "kdupdaterupdate.h"
#include "kdupdatertarget.h"
#include "kdupdaterupdatesourcesinfo.h"
#include "kdupdaterfiledownloader.h"
#include "kdupdaterfiledownloaderfactory.h"
#include "kdupdaterupdateoperations_p.h"
#include "kdupdaterupdateoperationfactory.h"

#include <QtCore/QDate>
#include <QtCore/QFile>
#include <QtCore/QMap>
#include <QtCore/QVariant>

/*!
   \ingroup kdupdater
   \class KDUpdater::Update kdupdaterupdate.h KDUpdaterUpdate
   \brief Represents a single update

   The KDUpdater::Update class contains information and mechanisms to download one update. It is
   created by KDUpdater::UpdateFinder and is used by KDUpdater::UpdateInstaller to download the UpdateFile
   corresponding to the update.

   The class makes use of appropriate network protocols (HTTP, HTTPS, FTP, or Local File Copy) to
   download the UpdateFile.

   The constructor of the KDUpdater::Update class is made protected, because it can be instantiated only by
   KDUpdater::UpdateFinder (which is a friend class). The destructor however is public.
*/

/*!
   \property KDUpdater::Update::canDownload

   This property contains whether the update can be downloaded or not. If the property is false, the URL scheme
   might not be supported.

   Get this property's value using %canDownload().
*/

/*!
   \property KDUpdater::Update::type

   This property contains the type of the update.

   Get this property's value using %type().
*/

/*!
   \property KDUpdater::Update::updateUrl

   This property contains the complete URL of the UpdateFile downloaded by this update.

   Get this property's value using %updateUrl().
*/

/*!
   \property KDUpdater::Update::releaseDate

   This property contains the release date of the update.

   Get this property's value using %releaseDate().
*/

/*!
   \property KDUpdater::Update::downloaded

   This property contains whether the update has been downloaded or not.
   If this property is true, you can get the complete name of the downloaded UpdateFile in #downloadedFileName.

   Get this property's value using %isDownloaded().

   \note The downloaded UpdateFile will be deleted when this class is destroyed
*/

/*!
   \property KDUpdater::Update::downloadedFileName

   This property contains the name of the downloaded UpdateFile after the download is complete, i.e. when downloaded is true.

   Get this property's value using %downloadedFileName().
*/

/*!
   \property KDUpdater::Update::compressedSize

   This property contains the compressed size of this update's data file.

   Get this property's value using %compressedSize().
*/

/*!
   \property KDUpdater::Update::uncompressedSize

   This property contains the uncompressed size of this update's data file.

   Get this property's value using %uncompressedSize().
*/

using namespace KDUpdater;

class Update::Private
{
public:
    Private( Update* qq )
        : q( qq ),
          target( 0 ),
        compressedSize( 0 ),
        uncompressedSize( 0 )
    {
    }

    void downloadProgress( int );
    void downloadAborted( const QString& msg );
    void downloadCompleted();

private:
    Update* const q;

public:
    Target * target;
    UpdateSourceInfo sourceInfo;
    QMap<QString, QVariant> data;
    QUrl updateUrl;
    UpdateType type;
    QList<UpdateOperation*> operations;
    QByteArray sha1sum;

    quint64 compressedSize;
    quint64 uncompressedSize;

    FileDownloader* fileDownloader;
};


/*!
   \internal
*/
Update::Update( Target* target, const UpdateSourceInfo& sourceInfo,
                          UpdateType type, const QUrl& updateUrl, const QMap<QString, QVariant>& data, quint64 compressedSize, quint64 uncompressedSize, const QByteArray& sha1sum )
    : Task(QLatin1String( "Update" ), Stoppable, target),
      d( new Private( this ) )
{
    d->target = target;
    d->sourceInfo = sourceInfo;
    d->data = data;
    d->updateUrl = updateUrl;
    d->type = type;

    d->compressedSize = compressedSize;
    d->uncompressedSize = uncompressedSize;
    d->sha1sum = sha1sum;

    d->fileDownloader = FileDownloaderFactory::instance().create( updateUrl.scheme(), this );
    if(d->fileDownloader)
    {
        d->fileDownloader->setUrl(d->updateUrl);
        d->fileDownloader->setSha1Sum( d->sha1sum );
        connect(d->fileDownloader, SIGNAL(downloadProgress(int)), this, SLOT(downloadProgress(int)));
        connect(d->fileDownloader, SIGNAL(downloadAborted(QString)), this, SLOT(downloadAborted(QString)) );
        connect(d->fileDownloader, SIGNAL(downloadCanceled()), this, SIGNAL(stopped()));
        connect(d->fileDownloader, SIGNAL(downloadCompleted()), this, SIGNAL(finished()));
    }

    switch( type ) {
    case NewPackage:
    case PackageUpdate:
    {
        QStringList args;
        args << data.value( QLatin1String( "Name" ) ).toString()
             << data.value( QLatin1String( "Version" ) ).toString()
             << data.value( QLatin1String( "ReleaseDate" ) ).toString();
        UpdateOperation * const packageOperation = UpdateOperationFactory::instance().create( QLatin1String( "UpdatePackage" ), args, target );
        d->operations.append( packageOperation );
        break;
    }
    case CompatUpdate:
    {
        QStringList args;
        args << data.value( QLatin1String( "CompatLevel" ) ).toString();
        UpdateOperation * const compatOperation = UpdateOperationFactory::instance().create( QLatin1String( "UpdateCompatLevel" ), args, target );
        d->operations.append( compatOperation );
        break;
    }
    default:
        break;
    }
}

/*!
   Destructor
*/
Update::~Update()
{
    const QString fileName = this->downloadedFileName();
    if( !fileName.isEmpty() )
        QFile::remove( fileName );
    qDeleteAll( d->operations );
    d->operations.clear();
}

/*!
   Returns the target for which this class is downloading the UpdateFile
*/
Target* Update::target() const
{
    return d->target;
}

/*!
   Returns the release date of the update downloaded by this class
*/
QDate Update::releaseDate() const
{
    return d->data.value( QLatin1String( "ReleaseDate" ) ).toDate();
}

/*!
   Returns data whose name is given in parameter, or an invalid QVariant if the data doesn't exist.
*/
QVariant Update::data( const QString& name ) const
{
    if ( d->data.contains( name ) )
        return d->data.value( name );
    return QVariant();
}

QUrl Update::updateUrl() const
{
    return d->updateUrl;
}

/*!
   Returns the update source info on which this update was created.
*/
UpdateSourceInfo Update::sourceInfo() const
{
    return d->sourceInfo;
}

UpdateType Update::type() const
{
    return d->type;
}

bool Update::canDownload() const
{
    return d->fileDownloader && d->fileDownloader->canDownload();
}

bool Update::isDownloaded() const
{
    return d->fileDownloader && d->fileDownloader->isDownloaded();
}

QString Update::downloadedFileName() const
{
    if(d->fileDownloader)
        return d->fileDownloader->downloadedFileName();

    return QString();
}

/*!
   \internal
*/
void Update::Private::downloadProgress( int pc )
{
    q->reportProgress( pc, tr( "Downloading update..." ) );
}

/*!
   \internal
*/
void Update::Private::downloadCompleted()
{
    q->reportProgress( 100, tr( "Update downloaded" ) );
    q->reportDone();
}

/*!
   \internal
*/
void Update::Private::downloadAborted( const QString& msg )
{
    q->reportError( msg );
}

/*!
   \internal
*/
void Update::doRun()
{
    if(d->fileDownloader)
        d->fileDownloader->download();
}

/*!
   \internal
*/
bool Update::doStop()
{
    if(d->fileDownloader)
        d->fileDownloader->cancelDownload();
    return true;
}

/*!
   \internal
*/
bool Update::doPause()
{
    return false;
}

/*!
   \internal
*/
bool Update::doResume()
{
    return false;
}

/*!
   Returns a list of operations needed by this update. For example, package update needs to change
   the package version, compat update needs to change the compat level...
 */
QList<UpdateOperation*> Update::operations() const
{
    return d->operations;
}

quint64 Update::compressedSize() const
{
    return d->compressedSize;
}

quint64 Update::uncompressedSize() const
{
    return d->uncompressedSize;
}

#include "moc_kdupdaterupdate.cpp"
