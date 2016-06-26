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

#include "KDUpdater/kdupdaterfiledownloader.h"
#include "KDUpdater/kdupdaterfiledownloaderfactory.h"
#include "ui_filedownloadmonitor.h"

#include <QSignalSpy>
#include <QTest>
#include <QFile>
#include <QPushButton>
#include <QDir>
#include <QBasicTimer>
#include <QApplication>

static const int TIMEOUT = 60*1000; // 60sec

Q_DECLARE_METATYPE(KDUpdater::FileDownloader*)

class FileDownloaderTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void shouldDownloadFile_data();
    void shouldDownloadFile();

};

class FileDownloadMonitor : public QDialog, public Ui::FileDownloadMonitor
{
    Q_OBJECT

public:
    FileDownloadMonitor(QWidget* parent=0)
        : QDialog(parent), m_timedout( false )
    {
        setupUi(this);
        m_timeoutTimer.start( TIMEOUT, this );
    }

    bool timedOut() const { return m_timedout; }

public Q_SLOTS:
    void downloadProgress(int percent) {
        progressBar->setValue(percent);
    }

    void downloadStarted() {
        textEdit->append( QLatin1String( "Download started" ) );
        buttonBox->setEnabled(false);
    }

    void downloadCompleted() {
        textEdit->append( QLatin1String( "Download completed" ) );
        buttonBox->setEnabled(true);
        prepareToClose();
    }

    void downloadCanceled() {
        textEdit->append( QLatin1String( "Download canceled" ) );
        buttonBox->setEnabled(true);
        prepareToClose();
    }

    void downloadAborted(const QString& errorMessage) {
        textEdit->append( QString::fromLatin1("Download aborted: %1").arg(errorMessage) );
        buttonBox->setEnabled(true);
        prepareToClose();
    }

protected:
    void prepareToClose() {
        if ( qgetenv( "KDUPDATER_TEST_SLOWLY" ).trimmed().isEmpty() ) {
            close();
        } else {
            textEdit->append( QLatin1String( "Closing the dialog box in 2 seconds" ) );
            m_closeTimer.start( 2000, this );
        }
    }

    void timerEvent( QTimerEvent * e ) {
        buttonBox->setEnabled(true);
        buttonBox->button(QDialogButtonBox::Ok)->click();
        if ( m_timeoutTimer.timerId() == e->timerId() )
            m_timedout = true;
    }

private:
    QBasicTimer m_closeTimer, m_timeoutTimer;
    bool m_timedout;
};

void FileDownloaderTest::initTestCase()
{
    qRegisterMetaType<KDUpdater::FileDownloader*>();
}

void FileDownloaderTest::shouldDownloadFile_data()
{
    QTest::addColumn<KDUpdater::FileDownloader*>("downloader");
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<bool>("canDownload");
    QTest::addColumn<bool>("downloadFail");
    QTest::addColumn<QString>("downloadError");

    KDUpdater::FileDownloader *downloader = 0;

    const QDir kdtools( QLatin1String( KDTOOLS_BASE ) );

    QTest::newRow("LocalFileDownloader") << downloader
                                         << QUrl::fromLocalFile( kdtools.absoluteFilePath( QLatin1String( "kdtools.pro" ) ) )
                                         << true << false << QString();

    QTest::newRow("FtpFileDownloader") << downloader
                                       << QUrl( QLatin1String( "ftp://ftp.trolltech.com/qt/pdf/whitepaper.pdf" ) )
                                       << true << false << QString();

    QTest::newRow("HttpFileDownloader") << downloader
                                        << QUrl( QLatin1String( "http://www.kdab.com/" ) )
                                        << true << false << QString();


    QTest::newRow("LocalFileDownloader, wrong file") << downloader
                                                     << QUrl::fromLocalFile( QLatin1String( "./foo.txt" ) )
                                                     << false << true << "Cannot open source file for reading.";

    QTest::newRow("FtpDownloader, wrong url") << downloader
                                              << QUrl( QLatin1String( "ftp://ftp.proxad.net/foo.txt" ) )
                                              << true << true << "Downloading file failed:\nFailed to open file.";

    if ( /*HTTPDOWNLOADER_3xx_HANDLING_FIXED?*/0 ) {
    QTest::newRow("HttpDownloader, wrong url") << downloader
                                               << QUrl( QLatin1String( "http://www.qtsoftware.com/foo.html" ) )
                                               << true << true << "Not Found";
    }

    QTest::newRow("HttpDownloader, wrong url II") << downloader
                                                  << QUrl( QLatin1String( "http://www.kdab.com/~christoph/foo.html" ) )
                                                  << true << true << "Not Found";
}

void FileDownloaderTest::shouldDownloadFile()
{
    QFETCH(KDUpdater::FileDownloader*, downloader);
    QFETCH(QUrl, url);
    QFETCH(bool, canDownload);
    QFETCH(bool, downloadFail);
    QFETCH(QString, downloadError);

    downloader = KDUpdater::FileDownloaderFactory::instance().create( url.scheme() );

    FileDownloadMonitor monitor;

    connect( downloader, SIGNAL(downloadStarted()), &monitor, SLOT(downloadStarted()) );
    connect( downloader, SIGNAL(downloadCompleted()), &monitor, SLOT(downloadCompleted()) );
    connect( downloader, SIGNAL(downloadCanceled()), &monitor, SLOT(downloadCanceled()) );
    connect( downloader, SIGNAL(downloadAborted(QString)), &monitor, SLOT(downloadAborted(QString)) );
    connect( downloader, SIGNAL(downloadProgress(int)), &monitor, SLOT(downloadProgress(int)) );

    QSignalSpy errorSpy( downloader, SIGNAL(downloadAborted(QString)) );

    QCOMPARE( downloader->scheme(), url.scheme() );
    downloader->setUrl(url);
    QCOMPARE( downloader->url(), url );
    QCOMPARE( downloader->canDownload(), canDownload );
    QCOMPARE( downloader->isDownloaded(), false );
    QCOMPARE( downloader->downloadedFileName(), QString() );
    downloader->download();

    monitor.setWindowTitle( downloader->url().toString() );
    monitor.exec();

    QVERIFY( !monitor.timedOut() );

    const bool downloadSucceeded = !downloadFail;
    QCOMPARE( downloader->isDownloaded(), downloadSucceeded );

    if (downloadError.isEmpty()) {
        QCOMPARE(errorSpy.count(), 0);
    } else {
        QCOMPARE(errorSpy.count(), 1);
        QVERIFY( errorSpy.at(0).at(0).toString().contains( downloadError ) );
    }

    if (!canDownload || downloadFail) {
        QCOMPARE( downloader->downloadedFileName(), QString() );
        return;
    }

    QString downloadedFile = downloader->downloadedFileName();
    qDebug("Downloaded file: %s", qPrintable(downloadedFile));
    QCOMPARE( QFile::exists(downloadedFile), true );
    delete downloader;
    QCOMPARE( QFile::exists(downloadedFile), false );
}

QTEST_MAIN(FileDownloaderTest)

#include "main.moc"
