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

#ifndef __KDTOOLS_KDUPDATERFILEDOWNLOADER_P_H__
#define __KDTOOLS_KDUPDATERFILEDOWNLOADER_P_H__

#include "kdupdaterfiledownloader.h"
#include <QNetworkReply>

// these classes are not a part of the public API

namespace KDUpdater
{

    //TODO make it a KDJob once merged
    class HashVerificationJob : public QObject
    {
        Q_OBJECT
    public:
        enum Error {
            NoError=0,
            ReadError=128,
            SumsDifferError
        };
        
        explicit HashVerificationJob( QObject* parent=0 );
        ~HashVerificationJob();
        
        void setDevice( QIODevice* dev );
        void setSha1Sum( const QByteArray& data );
 
        bool hasError() const;
        int error() const;
        
        void start();
 
    Q_SIGNALS:
        void finished( KDUpdater::HashVerificationJob* );
 
    private:
        void emitFinished();
        void timerEvent( QTimerEvent* te ) KDAB_OVERRIDE;

    private:
        class Private;
        kdtools::pimpl_ptr<Private> d;
    };

    class LocalFileDownloader : public FileDownloader
    {
        Q_OBJECT

    public:
        explicit LocalFileDownloader(QObject* parent=0);
        ~LocalFileDownloader();

        bool canDownload() const;
        bool isDownloaded() const;
        QString downloadedFileName() const;
        LocalFileDownloader* clone( QObject* parent=0 ) const KDAB_OVERRIDE;

    public Q_SLOTS:
        void cancelDownload();

    protected:
        void timerEvent(QTimerEvent* te);
        void onError() KDAB_OVERRIDE;
        void onSuccess() KDAB_OVERRIDE;

    private Q_SLOTS:
        /* reimp */ void doDownload();

    private:
        struct Private;
        kdtools::pimpl_ptr<Private> d;
    };

    class ResourceFileDownloader : public FileDownloader
    {
        Q_OBJECT

    public:
        explicit ResourceFileDownloader(QObject* parent=0);
        ~ResourceFileDownloader();

        bool canDownload() const;
        bool isDownloaded() const;
        QString downloadedFileName() const;
        ResourceFileDownloader* clone( QObject* parent=0 ) const KDAB_OVERRIDE;

    public Q_SLOTS:
        void cancelDownload();

    protected:
        void timerEvent(QTimerEvent* te);
        void onError() KDAB_OVERRIDE;
        void onSuccess() KDAB_OVERRIDE;

    private Q_SLOTS:
        void doDownload() KDAB_OVERRIDE;

    private:
        struct Private;
        kdtools::pimpl_ptr<Private> d;
    };

#ifndef QT_NO_FTP
    class FtpDownloader : public FileDownloader
    {
        Q_OBJECT

    public:
        explicit FtpDownloader(QObject* parent=0);
        ~FtpDownloader();

        bool canDownload() const;
        bool isDownloaded() const;
        QString downloadedFileName() const;
        FtpDownloader* clone( QObject* parent=0 ) const KDAB_OVERRIDE;

    public Q_SLOTS:
        void cancelDownload();

    protected:
        void onError() KDAB_OVERRIDE;
        void onSuccess() KDAB_OVERRIDE;

    private Q_SLOTS:
        void doDownload() KDAB_OVERRIDE;
        void ftpDone(bool error);
        void ftpCmdStarted(int id);
        void ftpCmdFinished(int id, bool error);
        void ftpStateChanged(int state);
        void ftpDataTransferProgress(qint64 done, qint64 total);

    private:
        struct Private;
        kdtools::pimpl_ptr<Private> d;
    };
#endif // QT_NO_FTP

    class HttpDownloader : public FileDownloader
    {
        Q_OBJECT

    public:
        explicit HttpDownloader(QObject* parent=0);
        ~HttpDownloader();

        bool canDownload() const;
        bool isDownloaded() const;
        QString downloadedFileName() const;
        HttpDownloader* clone( QObject* parent=0 ) const KDAB_OVERRIDE;

    public Q_SLOTS:
        void cancelDownload();

    protected:
        void onError() KDAB_OVERRIDE;
        void onSuccess() KDAB_OVERRIDE;

    private Q_SLOTS:
        void doDownload() KDAB_OVERRIDE;
        void httpReadyRead();
        void httpReadProgress( qint64 done, qint64 total );
        void httpError( QNetworkReply::NetworkError );
        void httpDone( bool error );
        void httpReqFinished();

    private:

        class Private;
        kdtools::pimpl_ptr<Private> d;
    };
}

#endif
