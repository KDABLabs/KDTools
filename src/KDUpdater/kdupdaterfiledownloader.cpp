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

#include "kdupdaterfiledownloader_p.h"
#include "kdupdaterfiledownloaderfactory.h"

#include <KDToolsCore/KDAutoPointer>

#include <QFile>
#include <QFtp>
#include <QNetworkAccessManager>
#if QT_VERSION >= 0x040600
#include <QNetworkProxyFactory>
#endif
#include <QNetworkReply>
#include <QPointer>
#include <QUrl>
#include <QTemporaryFile>
#include <QFileInfo>
#include <QCryptographicHash>
#include <QThreadPool>
#include <QStringList>

using namespace KDUpdater;

static int calcProgress(qint64 done, qint64 total)
{
    return total ? done * Q_INT64_C(100) / total : 0 ;
}

QByteArray KDUpdater::calculateHash( QIODevice* device, QCryptographicHash::Algorithm algo ) {
    Q_ASSERT( device );
    QCryptographicHash hash( algo );
    QByteArray buffer;
    buffer.resize( 512 * 1024 );
    while ( true ) {
        const qint64 numRead = device->read( buffer.data(), buffer.size() );
        if ( numRead <= 0 )
            return hash.result();
        hash.addData( buffer.constData(), numRead );
    }
    return QByteArray(); // never reached
}

QByteArray KDUpdater::calculateHash( const QString& path, QCryptographicHash::Algorithm algo ) {
    QFile file( path );
    if ( !file.open( QIODevice::ReadOnly ) )
        return QByteArray();
    return calculateHash( &file, algo );
}

class HashVerificationJob::Private {
public:
    Private() : hash( QCryptographicHash::Sha1 ), error( HashVerificationJob::ReadError ), timerId( -1 ) {
    }
    
    QPointer<QIODevice> device;
    QByteArray sha1Sum;
    QCryptographicHash hash;
    HashVerificationJob::Error error;
    int timerId;
};

HashVerificationJob::HashVerificationJob( QObject* parent ) : QObject( parent ), d( new Private )
{
}

HashVerificationJob::~HashVerificationJob()
{
}

void HashVerificationJob::setDevice( QIODevice* dev )
{
    d->device = dev;
}

void HashVerificationJob::setSha1Sum( const QByteArray& sum )
{
    d->sha1Sum = sum;
}

int HashVerificationJob::error() const
{
    return d->error;
}

bool HashVerificationJob::hasError() const
{
    return d->error != NoError;
}

void HashVerificationJob::start()
{
    Q_ASSERT( d->device );
    d->timerId = startTimer( 0 );
}

void HashVerificationJob::emitFinished()
{
    emit finished( this );
    deleteLater();
}

void HashVerificationJob::timerEvent( QTimerEvent* )
{
    Q_ASSERT( d->timerId >= 0 );
    if ( d->sha1Sum.isEmpty() ) {
        killTimer( d->timerId );
        d->timerId = -1;
        d->error = NoError;
        d->device->close();
        emitFinished();
        return;
    }

    QByteArray buf;
    buf.resize( 128 * 1024 );
    const qint64 read = d->device->read( buf.data(), buf.size() );
    if ( read > 0 ) {
        d->hash.addData( buf.constData(), read );
        return;
    }

    d->error = d->hash.result() == d->sha1Sum ? NoError : SumsDifferError;
    killTimer( d->timerId );
    d->timerId = -1;
    emitFinished();
}

////////////////////////////////////////////////////////////////////////////
// KDUpdater::FileDownloader
////////////////////////////////////////////////////////////////////////////

/*!
   \internal
   \ingroup kdupdater
   \class KDUpdater::FileDownloader kdupdaterfiledownloader.h

   Base class for file downloaders used in KDUpdater. File downloaders are used by
   the KDUpdater::Update class to download update files. Each subclass of FileDownloader
   can download file from a specific category of sources (eg. local, ftp, http etc).

   This is an internal class, not a part of the public API. Currently we have three
   subclasses of FileDownloader
   \li LocalFileDownloader - downloads from the local file system
   \li FtpDownloader - downloads from a FTP site
   \li HttpDownloader - downloads from a HTTP site

   Usage

   \code
   KDUpdater::FileDownloader* downloader = new KDUpdater::(some subclass name)

   downloader->setUrl( url );
   downloader->setFollowRedirect( followRedirects )
   downloader->download();

// wait for downloadCompleted() signal

QString downloadedFile = downloader->downloadedFileName();
\endcode
*/

struct KDUpdater::FileDownloader::FileDownloaderData
{
    FileDownloaderData() : autoRemove( true ) {
    }
    
    QUrl url;
    QString scheme;
    QByteArray sha1Sum;
    QString errorString;
    bool autoRemove;
    bool followRedirect;
};


#ifdef QT_MAC_USE_COCOA
void kdupdaterEncourageQtToStartNSApp();
#endif

KDUpdater::FileDownloader::FileDownloader(const QString& scheme, QObject* parent)
    : QObject(parent),
      d( new FileDownloaderData )
{
    d->scheme = scheme;
    d->followRedirect = false;
#ifdef QT_MAC_USE_COCOA
    kdupdaterEncourageQtToStartNSApp();
#endif
}

KDUpdater::FileDownloader::~FileDownloader()
{
}

void KDUpdater::FileDownloader::setUrl(const QUrl& url)
{
    d->url = url;
}

QUrl KDUpdater::FileDownloader::url() const
{
    return d->url;
}

void KDUpdater::FileDownloader::setSha1Sum( const QByteArray& sum )
{
    d->sha1Sum = sum;
}

QByteArray KDUpdater::FileDownloader::sha1Sum() const
{
    return d->sha1Sum;
}

QString FileDownloader::errorString() const
{
    return d->errorString;
}

void FileDownloader::setDownloadAborted( const QString& error )
{
    d->errorString = error;
    emit downloadAborted( error );
}

void KDUpdater::FileDownloader::setDownloadCompleted( const QString& path )
{
    KDAutoPointer<HashVerificationJob> job( new HashVerificationJob );
    QFile* file = new QFile( path, job.get() );
    if ( !file->open( QIODevice::ReadOnly ) ) {
        emit downloadProgress( 100 );
        onError();
        setDownloadAborted( tr("Could not reopen downloaded file %1 for reading: %2").arg( path, file->errorString() ) );
        return;
    }

    job->setDevice( file );
    job->setSha1Sum( d->sha1Sum );
    connect( job.get(), SIGNAL(finished(KDUpdater::HashVerificationJob*)), this, SLOT(sha1SumVerified(KDUpdater::HashVerificationJob*)) );
    job.release()->start();
}

void KDUpdater::FileDownloader::sha1SumVerified( KDUpdater::HashVerificationJob* job )
{
    emit downloadProgress( 100 );
    if ( job->hasError() ) {
        onError();
        setDownloadAborted( tr("Cryptographic hashes do not match.") );
    }
    else {
        onSuccess();
        emit downloadCompleted();
    }
}

QString KDUpdater::FileDownloader::scheme() const
{
    return d->scheme;
}

void KDUpdater::FileDownloader::setAutoRemoveDownloadedFile(bool val)
{
    d->autoRemove = val;
}

void KDUpdater::FileDownloader::setFollowRedirects( bool val )
{
    d->followRedirect = val;
}

bool KDUpdater::FileDownloader::followRedirects() const
{
    return d->followRedirect;
}

bool KDUpdater::FileDownloader::isAutoRemoveDownloadedFile() const
{
    return d->autoRemove;
}

void KDUpdater::FileDownloader::download() {
    QMetaObject::invokeMethod( this, "doDownload", Qt::QueuedConnection );
}

void KDUpdater::FileDownloader::cancelDownload()
{
    // Do nothing
}



////////////////////////////////////////////////////////////////////////////
// KDUpdater::FileDownloader
////////////////////////////////////////////////////////////////////////////

/*
  Even though QFile::copy() does the task of copying local files from one place
  to another, I prefer to use the timer and copy one block of data per unit time.

  This is because, it is possible that the user of KDUpdater is simultaneously
  downloading several files. Sometimes in tandem with other file downloaders.
  If the local file that is being downloaded takes a long time; then that will
  hang the other downloads.

  On the otherhand, local downloads need not actually download the file. It can
  simply pass on the source file as destination file. At this moment however,
  I think the user of LocalFileDownloader will assume that the downloaded file
  can be fiddled around with without worrying about whether it would mess up
  the original source or not.
*/

struct KDUpdater::LocalFileDownloader::Private
{
    Private() : source(0), destination(0),
                                downloaded(false), timerId(-1) { }

    QFile* source;
    QTemporaryFile* destination;
    QString destFileName;
    bool downloaded;
    int timerId;
};

KDUpdater::LocalFileDownloader::LocalFileDownloader(QObject* parent)
    :KDUpdater::FileDownloader(QLatin1String( "file" ), parent),
     d ( new Private )
{
}

KDUpdater::LocalFileDownloader::~LocalFileDownloader()
{
    if( this->isAutoRemoveDownloadedFile() && !d->destFileName.isEmpty() )
        QFile::remove(d->destFileName);
}

bool KDUpdater::LocalFileDownloader::canDownload() const
{
    const QString localFile = url().toLocalFile();
    QFileInfo fi( localFile );
    return fi.exists() && fi.isReadable();
}

bool KDUpdater::LocalFileDownloader::isDownloaded() const
{
    return d->downloaded;
}

void KDUpdater::LocalFileDownloader::doDownload()
{
    // Already downloaded
    if( d->downloaded )
        return;

    // Already started downloading
    if( d->timerId >= 0 )
        return;

    // Open source and destination files
    QString localFile = this->url().toLocalFile();
    d->source = new QFile(localFile, this);
    d->destination = new QTemporaryFile(this);

    if( !d->source->open(QFile::ReadOnly) )
    {
        onError();
        setDownloadAborted(tr("Cannot open source file for reading."));
        return;
    }

    if( !d->destination->open() )
    {
        onError();
        setDownloadAborted(tr("Cannot open destination file for writing."));
        return;
    }

    // Start a timer and kickoff the copy process
    d->timerId = startTimer(0); // as fast as possible
    emit downloadStarted();
    emit downloadProgress(0);
}

QString KDUpdater::LocalFileDownloader::downloadedFileName() const
{
    return d->destFileName;
}

KDUpdater::LocalFileDownloader* KDUpdater::LocalFileDownloader::clone( QObject* parent ) const
{
    return new LocalFileDownloader( parent );
}

void KDUpdater::LocalFileDownloader::cancelDownload()
{
    if( d->timerId < 0 )
        return;

    killTimer( d->timerId );
    d->timerId = -1;

    onError();
    emit downloadCanceled();
}

void KDUpdater::LocalFileDownloader::timerEvent(QTimerEvent*)
{
    if( !d->source || !d->destination )
        return;

    const qint64 blockSize = 32768;
    QByteArray buffer;
    buffer.resize( blockSize );
    const qint64 numRead = d->source->read( buffer.data(), buffer.size() );
    qint64 toWrite = numRead;
    while ( toWrite > 0 ) {
        const qint64 numWritten = d->destination->write( buffer.constData() + numRead - toWrite, toWrite );
        if ( numWritten < 0 ) {
            killTimer( d->timerId );
            d->timerId = -1;
            onError();
            setDownloadAborted( tr("Writing to %1 failed: %2").arg( d->destination->fileName(), d->destination->errorString() ) );
            return;
        }
        toWrite -= numWritten;
    }

    if( numRead > 0 ) {
        emit downloadProgress( calcProgress(d->source->pos(), d->source->size()) );
        return;
    }
    
    d->destination->flush();

    killTimer( d->timerId );
    d->timerId = -1;

    setDownloadCompleted( d->destination->fileName() );
}

void LocalFileDownloader::onSuccess()
{
    d->downloaded = true;
    d->destFileName = d->destination->fileName();
    d->destination->setAutoRemove( false );
    d->destination->close();
    delete d->destination;
    d->destination = 0;
    delete d->source;
    d->source = 0;
}

void LocalFileDownloader::onError()
{
    d->downloaded = false;
    d->destFileName.clear();
    delete d->destination;
    d->destination = 0;
    delete d->source;
    d->source = 0;
}

struct KDUpdater::ResourceFileDownloader::Private
{
    Private()
        : downloaded( false ), 
          timerId( -1 )
    { 
    }

    QString destFileName;
    bool downloaded;
    int timerId;
};

KDUpdater::ResourceFileDownloader::ResourceFileDownloader(QObject* parent)
    :KDUpdater::FileDownloader(QLatin1String( "resource" ), parent),
     d ( new Private )
{
}

KDUpdater::ResourceFileDownloader::~ResourceFileDownloader()
{
}

bool KDUpdater::ResourceFileDownloader::canDownload() const
{
    QUrl url = this->url();
    url.setScheme( QString::fromLatin1( "file" ) );
    QString localFile = QString::fromLatin1( ":%1" ).arg( url.toLocalFile() );
    QFileInfo fi(localFile);
    return fi.exists() && fi.isReadable();
}

bool KDUpdater::ResourceFileDownloader::isDownloaded() const
{
    return d->downloaded;
}

void KDUpdater::ResourceFileDownloader::doDownload()
{
    // Already downloaded
    if( d->downloaded )
        return;

    // Already started downloading
    if( d->timerId >= 0 )
        return;

    // Open source and destination files
    QUrl url = this->url();
    url.setScheme( QString::fromLatin1( "file" ) );
    d->destFileName = QString::fromLatin1( ":%1" ).arg( url.toLocalFile() );
    
    // Start a timer and kickoff the copy process
    d->timerId = startTimer(0); // as fast as possible
    emit downloadStarted();
    emit downloadProgress(0);
}

QString KDUpdater::ResourceFileDownloader::downloadedFileName() const
{
    return d->destFileName;
}

KDUpdater::ResourceFileDownloader* KDUpdater::ResourceFileDownloader::clone( QObject* parent ) const
{
    return new ResourceFileDownloader( parent );
}

void KDUpdater::ResourceFileDownloader::cancelDownload()
{
    if( d->timerId < 0 )
        return;

    killTimer( d->timerId );
    d->timerId = -1;

    emit downloadCanceled();
}

void KDUpdater::ResourceFileDownloader::timerEvent(QTimerEvent*)
{
    killTimer( d->timerId );
    d->timerId = -1;
    setDownloadCompleted( d->destFileName );
}

void KDUpdater::ResourceFileDownloader::onSuccess()
{
    d->downloaded = true;
}

void KDUpdater::ResourceFileDownloader::onError()
{
    d->downloaded = false;
}

#ifndef QT_NO_FTP

////////////////////////////////////////////////////////////////////////////
// KDUpdater::FtpDownloader
////////////////////////////////////////////////////////////////////////////

struct KDUpdater::FtpDownloader::Private
{
    Private() : ftp(0), destination(0),
                          downloaded(false), ftpCmdId(-1), aborted(false) { }

    QFtp* ftp;
    QTemporaryFile* destination;
    QString destFileName;
    bool downloaded;
    int ftpCmdId;
    bool aborted;
};

KDUpdater::FtpDownloader::FtpDownloader(QObject* parent)
    : KDUpdater::FileDownloader(QLatin1String( "ftp" ), parent),
      d ( new Private )
{
}

KDUpdater::FtpDownloader::~FtpDownloader()
{
    if( this->isAutoRemoveDownloadedFile() && !d->destFileName.isEmpty() )
        QFile::remove(d->destFileName);
}

bool KDUpdater::FtpDownloader::canDownload() const
{
    // TODO: Check whether the ftp file actually exists or not.
    return true;
}

bool KDUpdater::FtpDownloader::isDownloaded() const
{
    return d->downloaded;
}

void KDUpdater::FtpDownloader::doDownload()
{
    if( d->downloaded )
        return;

    if( d->ftp )
        return;

    d->ftp = new QFtp(this);
    connect(d->ftp, SIGNAL(done(bool)), this, SLOT(ftpDone(bool)));
    connect(d->ftp, SIGNAL(commandStarted(int)), this, SLOT(ftpCmdStarted(int)));
    connect(d->ftp, SIGNAL(commandFinished(int,bool)), this, SLOT(ftpCmdFinished(int,bool)));
    connect(d->ftp, SIGNAL(stateChanged(int)), this, SLOT(ftpStateChanged(int)));
    connect(d->ftp, SIGNAL(dataTransferProgress(qint64,qint64)), this, SLOT(ftpDataTransferProgress(qint64,qint64)));

    d->ftp->connectToHost( url().host(), url().port(21) );
    d->ftp->login();
}

QString KDUpdater::FtpDownloader::downloadedFileName() const
{
    return d->destFileName;
}

KDUpdater::FtpDownloader* KDUpdater::FtpDownloader::clone( QObject* parent ) const
{
    return new FtpDownloader( parent );
}


void KDUpdater::FtpDownloader::cancelDownload()
{
    if( d->ftp )
    {
        d->aborted = true;
        d->ftp->abort();
    }
}

void KDUpdater::FtpDownloader::ftpDone(bool error)
{
    if( error )
    {
        const QString errorString = d->ftp->errorString();

        d->ftp->deleteLater();
        d->ftp = 0;
        d->ftpCmdId = -1;

        onError();
        
        if( d->aborted )
        {
            d->aborted = false;
            emit downloadCanceled();
        }
        else
            setDownloadAborted( errorString );
    }
    //PENDING what about the non-error case??
}

void KDUpdater::FtpDownloader::ftpCmdStarted(int id)
{
    if( id != d->ftpCmdId )
        return;

    emit downloadStarted();
    emit downloadProgress(0);
}

void KDUpdater::FtpDownloader::ftpCmdFinished(int id, bool error)
{
    if( id != d->ftpCmdId || error ) // PENDING why error -> return??
        return;

    disconnect(d->ftp, 0, this, 0);
    d->ftp->deleteLater();
    d->ftp = 0;
    d->ftpCmdId = -1;
    d->destination->flush();

    setDownloadCompleted( d->destination->fileName() );
}

void FtpDownloader::onSuccess()
{
    d->downloaded = true;
    d->destFileName = d->destination->fileName();
    d->destination->setAutoRemove( false );
    delete d->destination;
    d->destination = 0;

}

void FtpDownloader::onError()
{
    d->downloaded = false;
    d->destFileName.clear();
    delete d->destination;
    d->destination = 0;

}

void KDUpdater::FtpDownloader::ftpStateChanged(int state)
{
    switch(state)
    {
    case QFtp::Connected:
        // begin the download
        d->destination = new QTemporaryFile(this);
        d->destination->open(); //PENDING handle error
        d->ftpCmdId = d->ftp->get( url().path(), d->destination );
        break;
    case QFtp::Unconnected:
        // download was unconditionally aborted
        disconnect(d->ftp, 0, this, 0);
        d->ftp->deleteLater();
        d->ftp = 0;
        d->ftpCmdId = -1;
        onError();
        setDownloadAborted(tr("Download was aborted due to network errors."));
        break;
    }
}

void KDUpdater::FtpDownloader::ftpDataTransferProgress(qint64 done, qint64 total)
{
    emit downloadProgress( calcProgress(done,total) );
}

#endif // QT_NO_FTP

////////////////////////////////////////////////////////////////////////////
// KDUpdater::HttpDownloader
////////////////////////////////////////////////////////////////////////////

class KDUpdater::HttpDownloader::Private
{
public:
    explicit Private( HttpDownloader* qq ) : q( qq ), http(0), destination(0), downloaded(false),
                           aborted(false), retrying(false) { }

    HttpDownloader* const q;
    QNetworkAccessManager manager;
    QNetworkReply* http;
    QTemporaryFile* destination;
    QString destFileName;
    QStringList redirectList;
    bool downloaded;
    bool aborted;
    bool retrying;

    void shutDown() {
        disconnect( http, SIGNAL(finished()), q, SLOT(httpReqFinished()) );
        http->deleteLater();
        http = 0;
        destination->close();
        destination->deleteLater();
        destination = 0;

    }
};

KDUpdater::HttpDownloader::HttpDownloader(QObject* parent)
    : KDUpdater::FileDownloader(QLatin1String( "http" ), parent),
      d ( new Private( this ) )
{
}

KDUpdater::HttpDownloader::~HttpDownloader()
{
    if( this->isAutoRemoveDownloadedFile() && !d->destFileName.isEmpty() )
        QFile::remove(d->destFileName);
}

bool KDUpdater::HttpDownloader::canDownload() const
{
    // TODO: Check whether the ftp file actually exists or not.
    return true;
}

bool KDUpdater::HttpDownloader::isDownloaded() const
{
    return d->downloaded;
}

void KDUpdater::HttpDownloader::doDownload()
{
    if( d->downloaded )
        return;

    if( d->http )
        return;

    d->http = d->manager.get( QNetworkRequest( url() ) );   

    connect( d->http, SIGNAL(readyRead()), this, SLOT(httpReadyRead()) );
    connect( d->http, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(httpReadProgress(qint64,qint64)) );
    connect( d->http, SIGNAL(finished()), this, SLOT(httpReqFinished()) );
    connect( d->http, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(httpError(QNetworkReply::NetworkError)) );

    /*
    // In a future update, authentication should also be supported.

    connect(d->http, SIGNAL(proxyAuthenticationRequired(QNetworkProxy,QAuthenticator*)),
    this, SLOT(httpProxyAuth(QNetworkProxy,QAuthenticator*)));
    connect(d->http, SIGNAL(authenticationRequired(QString,QAuthenticator*)),
    this, SLOT(httpAuth(QString,QAuthenticator*)));
    */

    // Begin the download
    d->redirectList.push_back( url().toString() );
    d->destination = new QTemporaryFile(this);
    if ( !d->destination->open() ) {
        const QString err = d->destination->errorString();
        d->shutDown();
        setDownloadAborted( tr("Cannot download %1: Could not create temporary file: %2").arg( url().toString(), err ) );
        return;
    }
}

QString KDUpdater::HttpDownloader::downloadedFileName() const
{
    return d->destFileName;
}

KDUpdater::HttpDownloader* KDUpdater::HttpDownloader::clone( QObject* parent ) const
{
    return new HttpDownloader( parent );
}

void KDUpdater::HttpDownloader::httpReadyRead()
{
    static QByteArray buffer( 16384, '\0' );
    while( d->http->bytesAvailable() )
    {
        const qint64 read = d->http->read( buffer.data(), buffer.size() );
        qint64 written = 0;
        while( written < read ) {
            const qint64 numWritten = d->destination->write( buffer.data() + written, read - written );
            if ( numWritten < 0 ) {
                const QString err = d->destination->errorString();
                d->shutDown();
                setDownloadAborted( tr("Cannot download %1: Writing to temporary file failed: %2").arg( url().toString(), err ) );
                return;
            }
            written += numWritten;
        }
    }
}

void KDUpdater::HttpDownloader::httpError( QNetworkReply::NetworkError )
{
    static bool setProxySettings = false;
    if( !d->retrying && !setProxySettings )
    {
        d->shutDown();
        d->retrying = true;
        setProxySettings = true;

#if QT_VERSION >= 0x040600 && !defined(QT_NO_NETWORKPROXY)
        // silently force retry with global proxy settings
        QNetworkProxyFactory::setUseSystemConfiguration( true );
#endif
        
        doDownload();
        return;
    }
    httpDone( true );
}

void KDUpdater::HttpDownloader::cancelDownload()
{
    d->aborted = true;
    if( d->http )
    {
        d->http->abort();
        httpDone( true );
    }
}

void KDUpdater::HttpDownloader::httpDone( bool error )
{
    if( error )
    {
        QString err;
        if( d->http )
        {
            err = d->http->errorString();
            d->http->deleteLater();
            d->http = 0;
            onError();
        }

        if( d->aborted )
        {
            d->aborted = false;
            emit downloadCanceled();
        }
        else
            setDownloadAborted( err );
    }
    //PENDING: what about the non-error case??
}

void KDUpdater::HttpDownloader::onError()
{
    d->downloaded = false;
    d->destFileName.clear();
    delete d->destination;
    d->destination = 0;
}

void KDUpdater::HttpDownloader::onSuccess()
{
    d->downloaded = true;
    d->destFileName = d->destination->fileName();
    d->destination->setAutoRemove( false );
    delete d->destination;
    d->destination = 0;
}

void KDUpdater::HttpDownloader::httpReqFinished()
{
    const QVariant redirect = d->http == 0 ? QVariant() : d->http->attribute( QNetworkRequest::RedirectionTargetAttribute );
    const QUrl redirectUrl = redirect.toUrl();

    if ( followRedirects() && redirectUrl.isValid() )
    {
        if ( d->redirectList.contains( redirectUrl.toString() ) )
        {
            // redirected in an infinte loop or no new url thats an error
            d->destination->remove();
            setDownloadAborted( tr( "Not Found" ) );
        }
        else
        {
            d->redirectList.push_back( redirectUrl.toString() );
            // clean the previous download
            d->http->deleteLater();
            d->http = 0;
            d->destination->close();
            d->destination->deleteLater();
            d->destination = 0;

            d->http = d->manager.get( QNetworkRequest( redirectUrl ) );

            connect( d->http, SIGNAL(readyRead()), this, SLOT(httpReadyRead()) );
            connect( d->http, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(httpReadProgress(qint64,qint64)) );
            connect( d->http, SIGNAL(finished()), this, SLOT(httpReqFinished()) );
            connect( d->http, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(httpError(QNetworkReply::NetworkError)) );

            // Begin the download
            d->destination = new QTemporaryFile(this);
            d->destination->open(); //PENDING handle error
        }
    }
    else
    {
        if( d->http == 0 )
            return;
        httpReadyRead();
        d->destination->flush();
        setDownloadCompleted( d->destination->fileName() );
        d->http->deleteLater();
        d->http = 0;
    }
}

void KDUpdater::HttpDownloader::httpReadProgress( qint64 done, qint64 total)
{
    emit downloadProgress( calcProgress( done, total ) );
}
