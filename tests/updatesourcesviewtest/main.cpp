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
#include <KDUpdater/UpdateSourcesInfo>
#include <KDUpdater/UpdateSourcesView>
#include <QTest>
#include <QVBoxLayout>

#include "../kdupdater_common.cpp"

class UpdateSourcesViewTest : public KDUpdaterTest
{
    Q_OBJECT

private Q_SLOTS:
    void testCase1();
};

void UpdateSourcesViewTest::testCase1()
{
    setApplicationDirectory( QLatin1String( "normal" ) );

    QWidget* widget = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout(widget);

    KDUpdater::UpdateSourcesView* view1 = new KDUpdater::UpdateSourcesView(widget);
    view1->setUpdateSourcesInfo( target.updateSourcesInfo() );
    layout->addWidget( view1 );

    KDUpdater::UpdateSourcesView* view2 = new KDUpdater::UpdateSourcesView(widget);
    view2->setUpdateSourcesInfo( target.updateSourcesInfo() );
    layout->addWidget( view2 );

    widget->resize(640, 480);
    widget->show();

    waitForWindowShown( widget );

    tearDownApplicationDirectory( QLatin1String( "normal" ) );
}

QTEST_MAIN(UpdateSourcesViewTest)

#include "main.moc"
