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

#include "kdupdaterufcompressor.h"
#include "kdupdaterufuncompressor_p.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QTest>

class CompressTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testCase1();
    void testCase2();
    void cleanupTestCase();

private:
    QString m_sourceDir;
    QString m_zipFile;
};

void CompressTest::initTestCase()
{
    m_sourceDir = QLatin1String( "src" );
    m_zipFile = QLatin1String( "compressed.kvz" );
}

void CompressTest::testCase1()
{
    const QString srcPath = QString::fromLatin1( "%1/../../%2" ).arg( qApp->applicationDirPath(),
                                                                      m_sourceDir );

    KDUpdater::UFCompressor compressor;
    compressor.setFileName( m_zipFile );
    compressor.setSource( srcPath );
    compressor.compress();

    QFile file( m_zipFile );
    QVERIFY( file.size() != 0 );
}

void CompressTest::testCase2()
{
    const QLatin1String destDir( "." );

    KDUpdater::UFUncompressor uncompressor;
    uncompressor.setFileName( m_zipFile );
    uncompressor.setDestination( destDir );
    uncompressor.uncompress();
    QVERIFY( uncompressor.errorString().isEmpty() );
}

static bool removeDirectory( const QString& path )
{
    const QFileInfoList entries = QDir( path ).entryInfoList( QDir::NoDotAndDotDot | QDir::AllEntries | QDir::Hidden );
    Q_FOREACH ( const QFileInfo& fi, entries  ) {
        if ( fi.isDir() && !fi.isSymLink() ) {
            if ( !removeDirectory( fi.filePath() ) ) {
                return false;
            }
        } else {
            QFile f( fi.filePath() );
            f.setPermissions( f.permissions() | QFile::WriteOwner | QFile::WriteUser );
            if ( !f.remove() ) {
                return false;
            }
        }
    }
    return QDir().rmdir( path );
}

void CompressTest::cleanupTestCase()
{
    QVERIFY( QFile::remove( m_zipFile ) );
    QVERIFY( removeDirectory( m_sourceDir ) );
}

QTEST_MAIN(CompressTest)

#include "main.moc"
