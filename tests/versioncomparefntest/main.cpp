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
#include <KDUpdater/KDUpdater>
#include <QTest>

class VersionCompareFnTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testCase1();
    void testCase2();
    void testCase3();
};

void VersionCompareFnTest::testCase1()
{
    QCOMPARE( KDUpdater::compareVersion( QLatin1String("2.0"), QLatin1String("2.1") ), -1 );
    QCOMPARE( KDUpdater::compareVersion( QLatin1String("2.1"), QLatin1String("2.0") ), +1 );
    QCOMPARE( KDUpdater::compareVersion( QLatin1String("2.0"), QLatin1String("2.0") ),  0 );
    QCOMPARE( KDUpdater::compareVersion( QLatin1String("2.1"), QLatin1String("2.1") ),  0 );
}

void VersionCompareFnTest::testCase2()
{
    QCOMPARE( KDUpdater::compareVersion( QLatin1String("2.0"),   QLatin1String("2.x") ),   0 );
    QCOMPARE( KDUpdater::compareVersion( QLatin1String("2.x"),   QLatin1String("2.0") ),   0 );
    QCOMPARE( KDUpdater::compareVersion( QLatin1String("2.0.1"), QLatin1String("2.x") ),   0 );
    QCOMPARE( KDUpdater::compareVersion( QLatin1String("2.x"),   QLatin1String("2.0.1") ), 0 );
}

void VersionCompareFnTest::testCase3()
{
    QCOMPARE( KDUpdater::compareVersion( QLatin1String("2.0.12.4"), QLatin1String("2.1.10.4") ), -1 );
    QCOMPARE( KDUpdater::compareVersion( QLatin1String("2.0.12.x"), QLatin1String("2.0.x") ),     0 );
    QCOMPARE( KDUpdater::compareVersion( QLatin1String("2.1.12.x"), QLatin1String("2.0.x") ),    +1 );
    QCOMPARE( KDUpdater::compareVersion( QLatin1String("2.1.12.x"), QLatin1String("2.x") ),       0 );
    QCOMPARE( KDUpdater::compareVersion( QLatin1String("2.x"),      QLatin1String("2.1.12.x") ),  0 );
}

QTEST_MAIN(VersionCompareFnTest)

#include "main.moc"
