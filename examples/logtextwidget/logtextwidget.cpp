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

#include <KDToolsGui/KDLogTextWidget>

#include <QBasicTimer>
#include <QTime>
#include <QApplication>
#include <QUuid>
#include <QHBoxLayout>

class MessageGenerator : public QObject {
    Q_OBJECT
public:
    explicit MessageGenerator( QObject * parent=0 );
    ~MessageGenerator();

Q_SIGNALS:
    void message( const QString & str, const QColor & col );

protected:
    void timerEvent( QTimerEvent * );

private:
    QBasicTimer timer;
    int count;
};

int main( int argc, char * argv[] ) {
    QApplication app( argc, argv );

    QWidget top;
    QHBoxLayout hlay( &top );
    hlay.setMargin( 0 );

    KDLogTextWidget widget;
    widget.setMinimumVisibleColumns( 40 );
    widget.setMinimumVisibleLines( 10 );
    widget.setAlternatingRowColors( true );
    widget.resize( 400, 200 );

    hlay.addWidget( &widget );
    top.show();

    MessageGenerator generator;

    QObject::connect( &generator, SIGNAL(message(QString,QColor)),
                      &widget, SLOT(message(QString,QColor)) );

    return app.exec();
}

#include "logtextwidget.moc"

MessageGenerator::MessageGenerator( QObject * p )
    : QObject( p ), timer(), count( 0 )
{
    timer.start( 500, this );
}

MessageGenerator::~MessageGenerator() {}

void MessageGenerator::timerEvent( QTimerEvent * e ) {
    static const Qt::GlobalColor colors[] = { Qt::red, Qt::green, Qt::blue, Qt::black, Qt::gray };
    if ( e->timerId() == timer.timerId() ) {
        emit message( QString::fromLatin1( "Message %1: %2" ).arg( count ).arg( QUuid::createUuid().toString() ),
                      colors[count % ( sizeof colors / sizeof *colors )] );
        ++count;
    } else {
        QObject::timerEvent( e );
    }
}
