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

#include <KDToolsGui/KDPropertyModel>
#include <KDToolsGui/KDPropertyView>
#include <KDToolsGui/KDIntProperty>
#include <KDToolsGui/KDDoubleProperty>


#include <QApplication>
#include <QScopedPointer>
#include <QSignalSpy>
#include <QTest>
#include <QDebug>

Q_DECLARE_METATYPE( KDPropertyInterface* )

class PropertyChangeTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testPropertyChange();

    void slotPropertyChange( KDPropertyInterface* prop )
    {
        qDebug() << "KDToolsTest::onModelPropertyChanged" << prop->name() << prop->value();
    }

};

void PropertyChangeTest::initTestCase()
{
    qRegisterMetaType<KDPropertyInterface*>( "KDPropertyInterface*" );
}

void PropertyChangeTest::testPropertyChange()
{	
	/* Set up model with some data */
    QScopedPointer<KDPropertyModel> model( new KDPropertyModel() );
    model->addProperty(new KDIntProperty(123, QLatin1String( "int property" )));
    model->addProperty(new KDDoubleProperty(123.123, QLatin1String( "double property" )));
    model->addProperty(new KDSizeProperty(QSize( 47, 11 ), QLatin1String( "Size property" )));

    connect( model.data(),  SIGNAL(propertyChanged(KDPropertyInterface*)),
             this, SLOT(slotPropertyChange(KDPropertyInterface*)) );

    Q_ASSERT_X(dynamic_cast<QApplication*>(qApp), __FUNCTION__, "QT_GUI_LIB not set? See docs for QTEST_MAIN");

    QScopedPointer<KDPropertyView> view(  new KDPropertyView() );
    view->setModel(model.data());
    view->show();
    QTest::qWaitForWindowShown( view.data() );

    // we need to activate the window if no window manager is present
    // also see https://bug-48637-attachments.webkit.org/attachment.cgi?id=72337 for a related bug
    view->activateWindow();
    QApplication::setActiveWindow(view.data());

    QSignalSpy spy( model.data(), SIGNAL(propertyChanged(KDPropertyInterface*)) );

    //qApp->exec();
    QTest::qWait( 100 ); // note: 100 seems to be enough to test reliable, lower values don't really seem to work

    QWidget* focusWidget = view->focusWidget();
    Q_ASSERT(focusWidget);

    /* Send events to edit a property and check that the propertyChanged signal is only
        emitted once */
    QTest::keyClicks( focusWidget, QLatin1String( "42" ) );
    QTest::keyClick( focusWidget, Qt::Key_Return );

    // Wait until signals are emitted
    QTest::qWait( 10 );

    QCOMPARE( spy.count(), 1 );
}


QTEST_MAIN(PropertyChangeTest)

#include "main.moc"
