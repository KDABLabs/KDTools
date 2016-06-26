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

#ifndef __KDTOOLSCORE_KDSIGNALSPY_H__
#define __KDTOOLSCORE_KDSIGNALSPY_H__

#include <KDToolsCore/kdtoolsglobal.h>
#include <KDToolsCore/pimpl_ptr.h>

#include <QtCore/QObject>

#include <QtCore/QList>
#include <QtCore/QVariant>
#include <QtCore/QByteArray>

QT_BEGIN_NAMESPACE
class QDebug;
template <typename T> class QVector;
QT_END_NAMESPACE

class KDTOOLSCORE_EXPORT KDSignalSpy : public QObject {
public:
    KDSignalSpy();
    explicit KDSignalSpy( QObject * object, int level=0 );
    explicit KDSignalSpy( const QList<QObject*> & objects, int level=0 );
    ~KDSignalSpy();

    bool addObject( QObject * object, int level=0 );
    bool addObjects( const QList<QObject*> & objects, int level=0 );

    struct Event;

    QVector<Event> events() const;
    QVector<Event> eventsForObject( const QObject * object ) const;

    void clearEvents();

    QDebug dumpEvents( QDebug stream ) const;
    QDebug dumpEvents( QDebug stream, const QObject * object ) const;

public:
    int qt_metacall( QMetaObject::Call call, int id, void * a[] ) KDAB_OVERRIDE;

private:
    class Private;
    kdtools::pimpl_ptr<Private> d;
};

#ifndef QT_NO_DEBUG_STREAM
KDTOOLSCORE_EXPORT QDebug operator<<( QDebug d, const KDSignalSpy & spy );
#endif

struct KDSignalSpy::Event {
    const QObject * object;
    QByteArray signal;
    QList<QVariant> arguments;
    const void * _reserved;
};

#endif /* __KDTOOLSCORE_KDSIGNALSPY_H__ */
