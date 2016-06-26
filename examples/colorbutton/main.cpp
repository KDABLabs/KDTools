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

#include <KDToolsGui/KDColorButton>

#include <QtGui>

int main( int argc, char * argv[] ) {
    QApplication app( argc, argv );

    QWidget widget;
    widget.setWindowTitle( QLatin1String( "KDColorButton Example" ) );

    QGridLayout glay( &widget );
    glay.setColumnStretch( 1, 1 );

    KDColorButton solid( Qt::green, &widget );
    solid.setAllowInvalidColors( false );

    QLabel solidLabel( QLatin1String( "Choose &opaque color:" ), &widget );
    solidLabel.setBuddy( &solid );

    QColor transparentGreen = Qt::green;
    transparentGreen.setAlpha( 127 );
    KDColorButton alpha( transparentGreen, &widget );
    QPalette pal = alpha.palette();
    pal.setBrush( alpha.backgroundRole(), KDColorButton::defaultBackgroundPixmap() );
    alpha.setPalette( pal );
    alpha.setAutoFillBackground( true );

    QLabel alphaLabel( QLatin1String( "Choose &transparent color:" ), &widget );
    alphaLabel.setBuddy( &alpha );

    glay.addWidget( &solidLabel, 0, 0 );
    glay.addWidget( &solid, 0, 1 );
    glay.addWidget( &alphaLabel, 1, 0 );
    glay.addWidget( &alpha, 1, 1 );

    widget.show();

    return app.exec();
}
