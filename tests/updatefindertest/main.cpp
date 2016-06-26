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

#include <KDUpdater/Target>
#include <KDUpdater/UpdateFinder>
#include <KDUpdater/Update>

#include <QApplication>
#include <QEventLoop>
#include <QSignalSpy>
#include <QTest>
#include <QTimer>
#include <QDir>
#include <QUrl>

#include "../kdupdater_common.cpp"

class UpdateFinderTest : public KDUpdaterTest
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testFinderIsAutodeleted();
    void testFinderSignalOrder();
    void testFinderIsStoppable();

    void testFinderResults_data();
    void testFinderResults();

public Q_SLOTS:
    void slotFinderStarted();
    void slotFinderStopped();
    void slotFinderFinished();

private:
    QStringList signalNames;
};

Q_DECLARE_METATYPE(QList<QUrl>)

void UpdateFinderTest::initTestCase()
{
    qRegisterMetaType<QList<QUrl> >();
}

void UpdateFinderTest::testFinderIsAutodeleted()
{
    setApplicationDirectory( QLatin1String( "normal" ) );

    QPointer<KDUpdater::UpdateFinder> finder = new KDUpdater::UpdateFinder(&target);
    connect( finder, SIGNAL(finished()), finder, SLOT(deleteLater()) );
    QVERIFY(finder!=0);
    QSignalSpy finished(finder, SIGNAL(finished()));
    finder->run();
    QCOMPARE(finished.count(), 1);

    // The finder is not deleted yet
    QVERIFY(finder!=0);
    // But when we reenter the event loop...
    QEventLoop loop;
    QTimer::singleShot( 0, &loop, SLOT(quit()) );
    loop.exec();
    // ### (marc) why does the following not suffice?
    //QCoreApplication::processEvents();
    // ... it goes away.
    QVERIFY(finder==0);

    tearDownApplicationDirectory( QLatin1String( "normal" ) );
}

void UpdateFinderTest::slotFinderStarted()
{
    signalNames <<  QLatin1String( "started" );
}

void UpdateFinderTest::slotFinderStopped()
{
    signalNames <<  QLatin1String( "stopped" );
}

void UpdateFinderTest::slotFinderFinished()
{
    signalNames <<  QLatin1String( "finished" );
}

void UpdateFinderTest::testFinderSignalOrder()
{
    setApplicationDirectory( QLatin1String( "normal" ) );

    KDUpdater::UpdateFinder *finder = new KDUpdater::UpdateFinder(&target);
    connect(finder, SIGNAL(started()), this, SLOT(slotFinderStarted()));
    connect(finder, SIGNAL(stopped()), this, SLOT(slotFinderStopped()));
    connect(finder, SIGNAL(finished()), this, SLOT(slotFinderFinished()));

    signalNames.clear();

    finder->run();

    QCOMPARE(signalNames, (QStringList() <<  QLatin1String( "started" )  << QLatin1String( "finished" ) ));
    QVERIFY(!finder->isRunning());
    QVERIFY(!finder->isStopped());
    QVERIFY(finder->isFinished());

    tearDownApplicationDirectory( QLatin1String( "normal" ) );
}

void UpdateFinderTest::testFinderIsStoppable()
{
    setApplicationDirectory( QLatin1String( "normal" ) );

    KDUpdater::UpdateFinder *finder = new KDUpdater::UpdateFinder(&target);
    connect(finder, SIGNAL(started()), this, SLOT(slotFinderStarted()));
    connect(finder, SIGNAL(stopped()), this, SLOT(slotFinderStopped()));
    connect(finder, SIGNAL(finished()), this, SLOT(slotFinderFinished()));

    // Schedule a call to stop() ASAP
    QMetaObject::invokeMethod( finder, "stop", Qt::QueuedConnection );

    signalNames.clear();

    finder->run();

    QCOMPARE(signalNames, (QStringList() <<  QLatin1String( "started" )  << QLatin1String( "stopped" ) ));
    QVERIFY(!finder->isRunning());
    QVERIFY(finder->isStopped());
    QVERIFY(!finder->isFinished());

    tearDownApplicationDirectory( QLatin1String( "normal" ) );
}

Q_DECLARE_METATYPE(KDUpdater::Error)

void UpdateFinderTest::testFinderResults_data()
{
    QTest::addColumn<QString>("dataSet");
    QTest::addColumn<KDUpdater::Error>("expectedError");
    QTest::addColumn<QString>("expectedErrorTextStart");
    QTest::addColumn<QStringList>("updateNames");
    QTest::addColumn<QStringList>("updateTitles");
    QTest::addColumn<QStringList>("updateVersions");
    QTest::addColumn<QList<QUrl> >("updateUrls");

    QStringList names;
    QStringList titles;
    QStringList versions;
    QList<QUrl> urls;
    names <<  QLatin1String( "TextFileSet" ) ;
    titles <<  QLatin1String( "License Text Update - GPL Version 3" ) ;
    versions <<  QLatin1String( "3.0" ) ;
    urls << QUrl( QLatin1String( "file:///" ) + QFile::decodeName( KDTOOLS_BASE ) + QLatin1String( "/tests/data/normal/updateserver/Update1.kvz" ) );

    QTest::newRow("normal") << "normal" << KDUpdater::ENoError << QString()
                            << names << titles << versions << urls;

    QTest::newRow("packages") << "packages" << KDUpdater::EUnknown << QString::fromLatin1("Parse error in")
                              << QStringList() << QStringList()
                              << QStringList() << QList<QUrl>();

    QTest::newRow("sourcedown") << "sourcedown" << KDUpdater::EUnknown << QString::fromLatin1("Could not download updates")
                                << QStringList() << QStringList()
                                << QStringList() << QList<QUrl>();

    QTest::newRow("updatesources") << "updatesources" << KDUpdater::EUnknown << QString::fromLatin1("XML Parse error in")
                                   << QStringList() << QStringList()
                                   << QStringList() << QList<QUrl>();

    QTest::newRow("updates") << "updates" << KDUpdater::EUnknown << QString::fromLatin1("Updates.Xml contains invalid content: PackageUpdate element without Version")
                             << QStringList() << QStringList()
                             << QStringList() << QList<QUrl>();
}

void UpdateFinderTest::testFinderResults()
{
    QFETCH(QString, dataSet);
    QFETCH(KDUpdater::Error, expectedError);
    QFETCH(QString, expectedErrorTextStart);
    QFETCH(QStringList, updateNames);
    QFETCH(QStringList, updateTitles);
    QFETCH(QStringList, updateVersions);
    QFETCH(QList<QUrl>, updateUrls);

    setApplicationDirectory(dataSet);

    KDUpdater::UpdateFinder *finder = new KDUpdater::UpdateFinder(&target);
    finder->run();

    QCOMPARE(finder->error(), static_cast<int>( expectedError));
    QVERIFY2(finder->errorString().startsWith(expectedErrorTextStart),
             QString::fromLatin1("Error text was: '%1'").arg(finder->errorString()).toLocal8Bit().constData());

    QList<KDUpdater::Update*> updates = finder->updates();

    if (expectedError!=KDUpdater::ENoError) {
        QCOMPARE(updates.size(), 0);
        tearDownApplicationDirectory( dataSet );
        return;
    }

    QCOMPARE(updates.size(), updateNames.size());

    for (int i=0; i<updateNames.size(); i++) {
        KDUpdater::Update *update = updates[i];

        QCOMPARE(update->data( QLatin1String( "Name" ) ).toString(), updateNames[i]);
        QCOMPARE(update->data( QLatin1String( "Description" ) ).toString(), updateTitles[i]);
        QCOMPARE(update->data( QLatin1String( "Version" ) ).toString(), updateVersions[i]);
#ifndef Q_WS_WIN
        if( update->updateUrl().scheme() == QLatin1String( "file" ) )
            QCOMPARE(update->updateUrl().toString().toUpper(), updateUrls[i].toString().toUpper());
        else
            QCOMPARE(update->updateUrl(), updateUrls[i]);
#else
        QCOMPARE(update->updateUrl(), updateUrls[i]);
#endif
    }

    tearDownApplicationDirectory( dataSet );
}

QTEST_MAIN(UpdateFinderTest)

#include "main.moc"
