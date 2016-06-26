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

#include <KDToolsCore/KDLog>
#include <KDToolsGui/KDLogTextWidget>

#include <QtGui>

int main( int argc, char * argv[] ) {

    QApplication app( argc, argv );

    KDCompositeLogDevice* cdev = new KDCompositeLogDevice();
    cdev->addLogDevice( new KDFileLogDevice( QLatin1String( "./test.log" ) ) );
    cdev->addLogDevice( new KDStderrLogDevice );
    cdev->addLogDevice( new KDSystemLogDevice( KDSystemLogDevice::User ) );

    QWidget top;
    QVBoxLayout vlay( &top );
    KDLogTextWidget widget( &top );
    vlay.addWidget( &widget );

    widget.setMinimumVisibleLines( 2 );

    KDSignalLogDevice* logObj = new KDSignalLogDevice();
    cdev->addLogDevice( logObj );
    QObject::connect( logObj, SIGNAL(info(QString)),
                      &widget, SLOT(message(QString)) );
    QObject::connect( logObj, SIGNAL(debug(QString)),
                      &widget, SLOT(message(QString))  );
    QObject::connect( logObj, SIGNAL(warning(QString)),
                      &widget, SLOT(message(QString)) );
    QObject::connect( logObj, SIGNAL(error(QString)),
                      &widget, SLOT(message(QString)) );

    top.show();

    KDLog log( cdev );
    log.setQDebugMessagesRedirected( true );

    qDebug("qDebug with messages redirected");
    qWarning("qWarning %d", 1 );
    qWarning("qWarning %d", 2 );
    qWarning("qWarning %d", 3 );

    log.setQDebugMessagesRedirected( false );
    qWarning("qWarning %d, only console", 4 ); // this one will appear on the console, if at all.

    log.logWarning( "Warning %d to KDLog", 5);
    log.logError( "Error %d to KDLog", 6);
    log.logInfo("Info %d to KDLog", 7 );
    widget.message( QObject::tr( "Red message" ), Qt::red );
    widget.message( QObject::tr( "Green message" ), Qt::green );
    widget.message( QObject::tr( "Blue message" ), Qt::blue );

    return app.exec();
}
