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
#include <QTimer>
#include <KDUpdater/Target>
#include <KDUpdater/PackagesInfo>
#include <KDUpdater/PackagesView>
#include <QTest>

#include "../kdupdater_common.cpp"

class PackagesViewTest : public KDUpdaterTest
{
    Q_OBJECT

private Q_SLOTS:
    void testCase1();

};

void PackagesViewTest::testCase1()
{
    setApplicationDirectory( QLatin1String( "normal" ) );

    KDUpdater::PackagesView * packagesView = new KDUpdater::PackagesView;
    packagesView->setPackageInfo( target.packagesInfo() );
    packagesView->show();
    packagesView->resize(640, 480);

    waitForWindowShown( packagesView );

    tearDownApplicationDirectory( QLatin1String( "normal" ) );
}

QTEST_MAIN(PackagesViewTest)

#include "main.moc"
