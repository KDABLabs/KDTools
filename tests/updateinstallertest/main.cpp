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
#include <KDUpdater/UpdateInstaller>
#include <KDUpdater/Update>

#include <QApplication>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QSignalSpy>
#include <QTest>
#include <QTimer>

#include "../kdupdater_common.cpp"

class UpdateInstallerTest : public KDUpdaterTest
{
    Q_OBJECT

private Q_SLOTS:
    void testInstallerIsAutodeleted();

    void testInstallerRun_data();
    void testInstallerRun();

private:
    QStringList signalNames;
};

void UpdateInstallerTest::testInstallerIsAutodeleted()
{
    setApplicationDirectory( QLatin1String( "normal" ) );

    QPointer<KDUpdater::UpdateFinder> finder = new KDUpdater::UpdateFinder(&target);
    connect( finder, SIGNAL(finished()), finder, SLOT(deleteLater()) );
    QSignalSpy finishedFinder( finder, SIGNAL(finished()) );
    finder->run();
    QCOMPARE( finishedFinder.count(), 1);

    QPointer<KDUpdater::UpdateInstaller> installer = new KDUpdater::UpdateInstaller(&target);
    connect( installer, SIGNAL(finished()), installer, SLOT(deleteLater()) );
    installer->setUpdatesToInstall(finder->updates());
    QVERIFY(installer!=0);
    QSignalSpy finished(installer, SIGNAL(finished()));
    installer->run();
    QCOMPARE(finished.count(), 1);

    // The finder is not deleted yet
    QVERIFY(installer!=0);
    // But when we reenter the event loop...
    QEventLoop loop;
    QTimer::singleShot( 0, &loop, SLOT(quit()) );
    loop.exec();
    // ### (marc) why does the following not suffice?
    //QCoreApplication::processEvents();
    // ... it goes away.
    QVERIFY(installer==0);

    tearDownApplicationDirectory( QLatin1String( "normal" ) );
}

void UpdateInstallerTest::testInstallerRun_data()
{
    QTest::addColumn<QString>("dataSet");
    QTest::addColumn<int>("expectedError");
    QTest::addColumn<QString>("expectedErrorTextStart");

    QTest::newRow("normal") << "normal" << 0 << QString();
    QTest::newRow("kvz") << "kvz" << static_cast<int>(KDUpdater::EUnknown) << "Couldn't uncompress";
    QTest::newRow("updateinstructions") << "updateinstructions" << static_cast<int>(KDUpdater::EUnknown) << "Cannot execute";
}

void UpdateInstallerTest::testInstallerRun()
{
    QFETCH(QString, dataSet);
    QFETCH(int, expectedError);
    QFETCH(QString, expectedErrorTextStart);

    setApplicationDirectory(dataSet);

    KDUpdater::UpdateFinder *finder = new KDUpdater::UpdateFinder(&target);
    finder->run();

    QCOMPARE(finder->error(), 0);

    QList<KDUpdater::Update*> updates = finder->updates();
    KDUpdater::UpdateInstaller *installer = new KDUpdater::UpdateInstaller(&target);
    installer->setUpdatesToInstall(updates);
    installer->run();

    QCOMPARE(installer->error(), expectedError);
    QVERIFY2(installer->errorString().startsWith(expectedErrorTextStart),
             QString::fromLatin1("Error text was: '%1'").arg(installer->errorString()).toLocal8Bit().constData());

    tearDownApplicationDirectory( dataSet );
}

QTEST_MAIN(UpdateInstallerTest)

#include "main.moc"
