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

#include <KDToolsGui/KDShortcutBar>
#include <QtGui>
#include <QSettings>

static void fillwidget( KDShortcutBar* w )
{
    QTreeView* t = new QTreeView;
    t->setModel( new QDirModel );
    t->setWindowTitle( QLatin1String( "Tree" ) );
    t->setWindowIcon( QApplication::style()->standardPixmap(QStyle::SP_ComputerIcon) );
    KDShortcut* cat = new KDShortcut( t );
    cat->setStatusIcon( QApplication::style()->standardPixmap(QStyle::SP_MessageBoxQuestion) );
    w->addShortcut( cat );
    QLabel* label;

    label = new QLabel( QLatin1String( "Label 1" ) );
    label->setWindowTitle( QLatin1String( "Shortcut 1" ) );
    label->setWindowIcon( QApplication::style()->standardPixmap(QStyle::SP_DriveCDIcon) );
    w->addShortcut( new KDShortcut( label ) );

    QPushButton* button = new QPushButton( QLatin1String( "Button 1" ) );
    button->setWindowTitle( QLatin1String( "Shortcut 2, with some long text, and it also has a status icon and a sub menu" ) );
    button->setWindowIcon( QApplication::style()->standardPixmap(QStyle::SP_DriveHDIcon) );
    cat = new KDShortcut( button );
    cat->setStatusIcon( QApplication::style()->standardPixmap(QStyle::SP_MessageBoxCritical) );
    QMenu* menu = new QMenu;
    menu->addAction( QLatin1String( "Item 1" ) );
    menu->addAction( QLatin1String( "Item 2" ) );
    menu->addAction( QLatin1String( "Item 3" ) );
    menu->addAction( QLatin1String( "Item 4" ) );
    menu->addAction( QLatin1String( "Item 5" ) );
    cat->setMenu( menu );
    w->addShortcut( cat );

    for( int i = 3; i < 12; ++i ) {
        label = new QLabel( QString::fromLatin1("Label %1").arg(i) );
        label->setWindowTitle(QString::fromLatin1("Shortcut %1").arg(i));
        label->setWindowIcon( QApplication::style()->standardPixmap(QStyle::SP_DriveNetIcon) );
        w->addShortcut( new KDShortcut( label ) );
    }

    label = new QLabel( QString::fromLatin1( "Label 12" ) );
    label->setWindowTitle( QString::fromLatin1( "Shortcut 12, only text" ) );
    w->addShortcut( new KDShortcut( label ) );
}

int main( int argc, char** argv ) {
    QApplication app( argc, argv );

    KDShortcutBar* w1 = new KDShortcutBar;
    KDShortcutBar* w2 = new KDShortcutBar;

    w1->setWidgetPlacement(KDShortcutBar::WidgetAbove);
    w2->setWidgetPlacement(KDShortcutBar::WidgetBelow);

    w1->setWindowTitle( QLatin1String( "KDShortcutBar: WidgetAbove Placement" ) );
    w2->setWindowTitle( QLatin1String( "KDShortcutBar: WidgetBelow Placement" ) );

    w2->setIconSize( QSize(32,32) );

    fillwidget(w1);
    fillwidget(w2);

    QSettings settings( QSettings::UserScope, QLatin1String( "kdab.net" ), QLatin1String( "kdshortcuttest" ) );
    w1->restoreState( qVariantValue<QByteArray>(settings.value( QLatin1String( "shortcutbar/state" ) )) );

    w1->show();
    w2->show();

    int rc = app.exec();
    settings.setValue(  QLatin1String( "shortcutbar/state" ) , w1->saveState() );
    return rc;
}
