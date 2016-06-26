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

#include <QApplication>
#include <KDUpdater/Target>
#include <KDUpdater/PackagesInfo>
#include <KDUpdater/UpdateSourcesInfo>
#include <QTest>
#include <QDebug>

#include "../kdupdater_common.cpp"

class UpdateClientTest : public KDUpdaterTest
{
    Q_OBJECT

private Q_SLOTS:
    void testCase1();
    void testCase2();
};

void UpdateClientTest::testCase1()
{
    KDUpdater::Target target;

    QCOMPARE(target.directory(), qApp->applicationDirPath());
    QCOMPARE(target.name(), qApp->applicationName());
    qDebug() << target.packagesXMLFileName();
    QCOMPARE(target.packagesXMLFileName(), QString::fromLatin1("%1/Packages.xml").arg(qApp->applicationDirPath()));
    QCOMPARE(target.updateSourcesXMLFileName (), QString::fromLatin1("%1/UpdateSources.xml").arg(qApp->applicationDirPath()));
    QVERIFY2(target.packagesInfo(), "Packages info was not created");
    QVERIFY2(target.updateSourcesInfo(), "Packages info was not created");
}

void UpdateClientTest::testCase2()
{
    setApplicationDirectory( QLatin1String( "normal" ) );

    KDUpdater::PackagesInfo* packages = target.packagesInfo();
    qDebug("Number of packages: %d", packages->packageInfoCount());
    for(int i=0; i<packages->packageInfoCount(); i++)
    {
        KDUpdater::PackageInfo info = packages->packageInfo(i);
        qDebug(" -->Package Name  : %s", qPrintable(info.name));
        qDebug("    Package Title : %s", qPrintable(info.title));
        qDebug("    Install Date  : %s", qPrintable(info.installDate.toString()));
    }

    KDUpdater::UpdateSourcesInfo* updateSources = target.updateSourcesInfo();
    qDebug("Number of update sources: %d", updateSources->updateSourceInfoCount());
    for(int i=0; i<updateSources->updateSourceInfoCount(); i++)
    {
        KDUpdater::UpdateSourceInfo info = updateSources->updateSourceInfo(i);
        qDebug(" -->Source Name   : %s", qPrintable(info.name));
        qDebug("    Source Title  : %s", qPrintable(info.title));
        qDebug("    URL           : %s", qPrintable(info.url.toString()));
    }

    tearDownApplicationDirectory( QLatin1String( "normal" ) );
}

QTEST_MAIN(UpdateClientTest)

#include "main.moc"
