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

#include <QObject>

#include <KDUpdater/Target>

#include <QTest>
#include <QFile>
#include <QDir>
#include <QString>
#include <QTextStream>

class KDUpdaterTest : public QObject {
protected:
    KDUpdaterTest() : QObject() {}

    void setApplicationDirectory( const QString & name );
    void tearDownApplicationDirectory( const QString & name );

    KDUpdater::Target target;

protected:
    void waitForWindowShown( QWidget * w ) const {
#if QT_VERSION >= 0x040600
        QTest::qWaitForWindowShown( w );
#else // code copied from qWaitForWindowShown() in 4.6:
#if defined(Q_WS_X11)
        extern void qt_x11_wait_for_window_manager(QWidget *w);
        qt_x11_wait_for_window_manager(window);
        QCoreApplication::processEvents();
#elif defined(Q_WS_QWS)
        Q_UNUSED(window);
        QTest::qWait(100);
#else
        Q_UNUSED(window);
        QTest::qWait(50);
#endif
#endif
        if ( qgetenv( "KDUPDATER_TEST_SLOWLY" ).trimmed().isEmpty() )
            return;
        QTest::qWait( 2000 );
    }

private:
    void copy( const QString & in, const QString & out ) {
        QFile::copy( in, out );
    }
    void preprocess( const QString & inFileName, const QString & outFileName ) {
        QFile in( inFileName );
        QFile out( outFileName );

        if ( !in.open( QIODevice::ReadOnly|QIODevice::Text ) )
            return;
        if ( !out.open( QIODevice::WriteOnly|QIODevice::Text ) )
            return;

        QTextStream i( &in ), o( &out );

        while ( !i.atEnd() )
            o << i.readLine()
#ifdef KDTOOLS_BASE
                .replace( QLatin1String( "@KDTOOLS_BASE@" ), QLatin1String( KDTOOLS_BASE ) )
#endif
#ifdef KDTOOLS_BUILD_DIR
                .replace( QLatin1String( "@KDTOOLS_BUILD_DIR@"  ), QLatin1String( KDTOOLS_BUILD_DIR  ) )
#endif
                ;
    }
};

static QString chopped( QString s, int num ) {
    s.chop( num );
    return s;
}

void KDUpdaterTest::setApplicationDirectory( const QString & name ) {

    tearDownApplicationDirectory( name );

    const QString suffix = QString::fromLatin1( "/tests/data/%2/appdir" ).arg( name );

    const QString appDirPath = QFile::decodeName( KDTOOLS_BASE ) + suffix;

    QDir origAppdir( QFile::decodeName( KDTOOLS_BASE ) + suffix );

    {
        QDir pwd;
        pwd.mkpath( name + QLatin1String( "/appdir_tmp" ) );
    }
    QDir tempAppdir( name + QLatin1String( "/appdir_tmp" ) );

    Q_FOREACH( const QString & fileName, origAppdir.entryList( QDir::Files ) ) {
        if ( fileName.endsWith( QLatin1String(".in") ) )
            preprocess( origAppdir.filePath( fileName ), chopped( tempAppdir.filePath( fileName ), 3 ) );
        else
            copy( origAppdir.filePath( fileName ), tempAppdir.filePath( fileName ) );
    }

    target.setDirectory( tempAppdir.absolutePath() );
}

void KDUpdaterTest::tearDownApplicationDirectory( const QString & name ) {

    QDir tempAppdir( name + QLatin1String( "/appdir_tmp" )  );

    if ( !tempAppdir.exists() )
        return;

    QDir pwd;

    Q_FOREACH( const QString & fileName, tempAppdir.entryList( QDir::Files ) )
        QFile::remove( tempAppdir.filePath( fileName ) );
    pwd.rmpath( name + QLatin1String( "/appdir_tmp" ) );

}
