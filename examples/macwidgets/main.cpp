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

//#include "helpbutton.h"
//#include "circularprogressindicator.h"

#include <QtGui/QApplication>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QMenu>
#include <QtGui/QPlastiqueStyle>
#include <QtGui/QWindowsStyle>
#include <QtGui/QListWidget>

#include <KDToolsGui/KDHelpButton>
#include <KDToolsGui/KDSearchLineEdit>
#include <KDToolsGui/KDCircularProgressIndicator>
#include <KDToolsGui/KDLockButton>
#include <KDToolsGui/KDListViewToolBar>

int main( int argc, char* argv[] )
{
    QApplication app( argc, argv );

    QWidget w;
    QGridLayout* const layout = new QGridLayout;
    w.setLayout( layout );

    KDSearchLineEdit edit1;
    edit1.setStyle( new QWindowsStyle );
    
    KDSearchLineEdit edit2;
    edit2.setStyle( new QPlastiqueStyle );
    
    KDSearchLineEdit edit3;

    KDHelpButton button1;
    button1.setStyle( new QWindowsStyle );

    KDHelpButton button2;
    button2.setStyle( new QPlastiqueStyle );
    
    KDHelpButton button3;

    
    QLabel label1;
    label1.setText( QObject::tr( "Windows Style" ) );
    QLabel label2;
    label2.setText( QObject::tr( "Plastique Style" ) );
    QLabel label3;
    label3.setText( QObject::tr( "Native Style" ) );

    //QObject::connect( &edit2, SIGNAL( textChanged( QString ) ), &edit3, SLOT( setText( QString ) ) );
    //QObject::connect( &edit3, SIGNAL( textChanged( QString ) ), &edit2, SLOT( setText( QString ) ) );

    //QObject::connect( &edit1, SIGNAL( textChanged( QString ) ), &app, SLOT( setBadgeLabel( QString ) ) );

    QMenu m;
    m.addAction( QObject::tr( "test" ) );
    m.addAction( QObject::tr( "test2" ) );
    edit1.setMenu( &m );
    edit2.setMenu( &m );
    edit3.setMenu( &m );
    edit3.setDefaultText( QObject::tr( "Filter..." ) );

    layout->addWidget( &label1, 0, 0 );
    layout->addWidget( &label2, 1, 0 );
    layout->addWidget( &label3, 2, 0 );

    layout->addWidget( &edit1, 0, 1 );
    layout->addWidget( &edit2, 1, 1 );
    layout->addWidget( &edit3, 2, 1 );

    layout->addWidget( &button1, 0, 2 );
    layout->addWidget( &button2, 1, 2 );
    layout->addWidget( &button3, 2, 2 );

    layout->addWidget( new QListWidget, 3, 0, 2, 3 );

    KDListViewToolBar toolBar;
    QAction* const actionAdd = new QAction( QObject::tr( "+" ), 0 );
    actionAdd->setToolTip( QObject::tr( "Add stuff..." ) );
    toolBar.addAction( actionAdd );

    QAction* const actionRemove = new QAction( QObject::tr( "-" ), 0 );
    actionRemove->setToolTip( QObject::tr( "Remove stuff..." ) );
    actionRemove->setEnabled( false );
    toolBar.addAction( actionRemove );
     
    toolBar.addSeparator();

    QAction* const actionOptions = new QAction( QObject::tr( "*" ), 0 );
    actionOptions->setMenu( &m );
    actionOptions->setToolTip( QObject::tr( "Options" ) );
    toolBar.addAction( actionOptions );
    layout->addWidget( &toolBar, 5, 0, 1, 3 );

    layout->addWidget( new KDCircularProgressIndicator, 2, 3 );
    
    KDLockButton lock;
    lock.setAutoLock( true );
    layout->addWidget( &lock, 6, 0, 1, 3 );

    w.show();

    return app.exec();
}
