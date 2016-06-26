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

#ifndef QT_NO_KEYWORDS
# define QT_NO_KEYWORDS // we use 'signals' as a variable name...
#endif

#include "kdsignalspy.h"

#include <QVariant>
#include <QByteArray>
#include <QPair>
#include <QMetaMethod>
#include <QDebug>
#include <QStringList>
#include <QMutex>

class KDSignalSpy::Private {
    friend class ::KDSignalSpy;
public:
    Private();
    ~Private();

private:
    void addSignal( const QObject *, const QMetaMethod &, const char * );
    void addEmission( int id, void * args[] );

private:
    // the full, normalized signal name
    int numSlots;
    QMap< const QObject*, QString > objectNameMap;
    QMap< const QObject*, QList<QByteArray> > signals;
    QList< QList<int> > signalArguments;
    QVector< QPair<const QObject*,QByteArray> > indexMap;
    QVector<Event> events;
    mutable QMutex mutex;
};

KDSignalSpy::Private::Private()
    : numSlots( 0 ),
      objectNameMap(),
      signals(),
      signalArguments(),
      indexMap(),
      events(),
      mutex()
{

}

KDSignalSpy::Private::~Private() {}

/*!
  \class KDSignalSpy KDSignalSpy
  \ingroup core
  \brief A recorder for signal emissions, across multiple QObjects.

  KDSignalSpy is clearly inspired by QSignalSpy, but can monitor
  multiple signals on multiple \link QObject QObjects\endlink using a
  single KDSignalSpy instance. It therefore records the relative order
  of signal emissions, and can answer questions like "Is \c signalA
  always emitted before \a signalB?"

  To use a KDSignalSpy, create an instance, and \link addObject add
  objects\endlink to it, let it record a while, and retrieve the
  signal emission log using events() afterwards. You can add as many
  objects as you like, and you can restrict event reporting to certain
  signals by using the \c level parameter of addObject() or
  addObjects().

  As a convenience, some member functions of KDSignalSpy have been
  made thread-safe. This is noted in the documentation of the
  function. In addition, it is safe to monitor objects from different
  threads simultaneously, turning KDSignalSpy into a convenient
  thread-synchronization debugger.

  \note It is not recommended that you derive KDSignalSpy further. The
  conditions under which this would be safe are complex and subject to
  change in new versions. Later versions of KDSignalSpy might be more
  well-defined in this area.
*/

/*!
  Constructs a KDSignalSpy. The instance does not listen to any
  objects yet.
*/
KDSignalSpy::KDSignalSpy()
    : QObject(), d( new Private )
{

}

/*!
  Constructs a KDSignalSpy listening to all signals decared on object
  \a o up to \a level levels deep.
*/
KDSignalSpy::KDSignalSpy( QObject * o, int level )
    : QObject(), d( new Private )
{
    addObject( o, level );
}

/*!
  Constructs a KDSignalSpy listening to all signals decared on objects
  \a o up to \a level levels deep.
*/
KDSignalSpy::KDSignalSpy( const QList<QObject*> & o, int level )
    : QObject(), d( new Private )
{
    addObjects( o, level );
}

KDSignalSpy::~KDSignalSpy() {}

/*!
  Returns the events recorded to far.

  This function is thread-safe.
*/
QVector<KDSignalSpy::Event> KDSignalSpy::events() const {
    const QMutexLocker locker( &d->mutex );
    return d->events;
}

/*!
  Returns the events recorded for object \a object so far.

  This function is thread-safe.
*/
QVector<KDSignalSpy::Event> KDSignalSpy::eventsForObject( const QObject * object ) const {
    const QMutexLocker locker( &d->mutex );
    QVector<Event> result;
    result.reserve( d->events.size() );
    Q_FOREACH( const Event & ev, d->events )
	if ( ev.object == object )
	    result.push_back( ev );
    return result;
}

/*!
  Clears the event log, but continues listening to all signals.

  This function is thread-safe.
*/
void KDSignalSpy::clearEvents() {
    const QMutexLocker locker( &d->mutex );
    d->events.clear();
}

/*!
  Adds \a objects to the list of objects tracked for signal
  emissions. See the description of addObject() for more
  information. Returns \c true on success, \c false if an object
  couldn't be added properly.

  \todo Make this atomic, ie. either we listen to all objects and
  return \c true, or to none of \a objects, and return \c false.
*/
bool KDSignalSpy::addObjects( const QList<QObject*> & objects, int level ) {
    Q_FOREACH( QObject * o, objects )
	if ( !addObject( o, level ) )
	    return false;
    return true;
}

/*!
  Add \a object to the list of objects tracked for signal
  emissions. Only signals declared in the \a level most-derived
  classes are tracked.

  Returns \c true if all requested signals could be connected to, \c
  false otherwise.

  This function is thread-safe. However, if \a o is deleted by another
  thread while this function runs, the program will likely crash.

  \sa addObjects()
*/
bool KDSignalSpy::addObject( QObject * o, int level ) {
#ifdef Q_CC_BOR
    const int myMethodOffset = QObject::staticMetaObject.methodCount();
#else
    static const int myMethodOffset = QObject::staticMetaObject.methodCount();
#endif
    if ( !o )
	return false;

    const QMutexLocker locker( &d->mutex );

    d->objectNameMap[o] = o->objectName();

    // iteration over levels of inhertiance, up to \a level deep:
    bool success = true;
    for ( const QMetaObject * mo = o->metaObject() ; mo && level >= 0 ; mo = mo->superClass(), --level ) {

	const int hisMethodOffset = mo->methodOffset();
	// iterations over meta methods, to find all available signals:
	for ( int i = hisMethodOffset, end = mo->methodCount() ; i < end ; ++i ) {

	    const QMetaMethod m = mo->method( i );

	    if ( m.methodType() != QMetaMethod::Signal )
		continue;

	    if ( QMetaObject::connect( o, i, this, d->numSlots + myMethodOffset, Qt::DirectConnection ) ) {
            d->addSignal( o, m, mo->className() );
            ++d->numSlots;
	    } else {
#if QT_VERSION >= 0x050000
            qWarning("KDSignalSpy: QMetaObject::connect returned false. Unable to connect to signal '%s' on object '%s'.",
                m.methodSignature().data(), qPrintable( o->objectName() ) );
#else
            qWarning("KDSignalSpy: QMetaObject::connect returned false. Unable to connect to signal '%s' on object '%s'.",
                m.signature(), qPrintable( o->objectName() ) );
#endif
            success = false;
	    }

	}

    }
    return success;
}

int KDSignalSpy::qt_metacall( QMetaObject::Call call, int id, void * args[] ) {

    const QMutexLocker locker( &d->mutex );

    id = QObject::qt_metacall( call, id, args );
    if ( id < 0 )
	return id; // was one of QObject's

    if ( call == QMetaObject::InvokeMetaMethod ) {
	if ( id < d->numSlots )
	    d->addEmission( id, args ); // is one of our's
	id -= d->numSlots;
    }
    return id;
}

void KDSignalSpy::Private::addSignal( const QObject * o, const QMetaMethod & m, const char * className ) {

#if QT_VERSION >= 0x050000
    const QByteArray signature = QByteArray( className ) + "::" + m.methodSignature();
#else
    const QByteArray signature = QByteArray( className ) + "::" + m.signature();
#endif

    signals[o].push_back( signature );
    indexMap.push_back( qMakePair( o, signature ) );

    QList<int> sa;
    const QList<QByteArray> params = m.parameterTypes();
    Q_FOREACH( const QByteArray & param, params ) {
	const int tp = QMetaType::type( param.data() );
	if ( tp == QMetaType::Void )
	    qWarning("Don't know how to handle '%s', use qRegisterMetaType to register it.", param.data() );
	sa.push_back( tp );
    }
    signalArguments.push_back( sa );
}

void KDSignalSpy::Private::addEmission( int id, void * args[] ) {
    QList<QVariant> list;
    const QList<int> & sa = signalArguments[id];
    for ( int i = 0, end = sa.size() ; i < end ; ++i )
	list.push_back( QVariant( static_cast<QMetaType::Type>( sa[i] ), args[i+1] ) );
    const QPair<const QObject*,QByteArray> pair = indexMap[id];
    const Event ev = { pair.first, pair.second, list, 0 };
    events.push_back( ev );
}

static QDebug & formatArgumentList( QDebug & s, const QList<QVariant> & list ) {
    if ( list.empty() )
	return s;
    s << ' ';
    bool first = true;
    Q_FOREACH( const QVariant & v, list ) {
	if ( !first )
	    s << ", ";
	s << v;
	s.nospace();
	first = false;
    }
    return s.nospace();
}

/*!
  Dumps the event log to \a s, and returns \a s.

*/
QDebug KDSignalSpy::dumpEvents( QDebug s ) const {

    s << "KDSignalSpy event dump" << endl;
    s.nospace();

    const QMutexLocker locker( &d->mutex );

    Q_FOREACH( const Event & ev, d->events ) {
	s << qPrintable( d->objectNameMap.value( ev.object ) )
	  << "->" << ev.signal.left( ev.signal.indexOf( '(' ) ).constData() << '(';
	formatArgumentList( s, ev.arguments ) << ')' << endl;
    }
    return s.space();
}

/*!
  Dumps \c eventsForObject( \a object ) to \a s, returning \a s.
*/
QDebug KDSignalSpy::dumpEvents( QDebug s, const QObject * object ) const {

    s << "KDSignalSpy event dump for" << object << endl;
    s.nospace();

    const QMutexLocker locker( &d->mutex );

    Q_FOREACH( const Event & ev, d->events ) {
	if ( ev.object != object )
	    continue;
	s << ev.signal.left( ev.signal.indexOf( '(' ) ).constData() << '(';
	formatArgumentList( s, ev.arguments ) << ')' << endl;
    }
    return s.space();
}

/*!
  \relates KDSignalSpy
  Dumps \a spy's event log to \a s, and returns \a s.
*/
QDebug operator<<( QDebug d, const KDSignalSpy & spy ) {
    return spy.dumpEvents( d );
}


/*!
  \class KDSignalSpy::Event
  \relates KDSignalSpy
  \brief KDSignalSpy event record

  The KDSignalSpy::Event structure contains the information pertaining
  to a single signal emission event. The information recorded is
  stored in the fields of the structure: .object, .signal, and
  .arguments.
*/

/*!
  \var KDSignalSpy::Event::object

  The object emitting the signal. Note that it is usually not a good
  idea to dereference this pointer, since the object might have been
  deleted by the time you retrieve the event.
*/

/*!
  \var KDSignalSpy::Event::signal

  The \link QMetaMethod::signature() signature\endlink of the signal
  that was emitted.
*/

/*!
  \var KDSignalSpy::Event::arguments

  The signal arguments, packaged as a list of \link QVariant
  QVariants\endlink.
*/

#ifdef KDTOOLSCORE_UNITTESTS

#include <KDUnitTest/Test>

#include <QSignalMapper>
#include <QDebug>

QT_BEGIN_NAMESPACE
    static inline std::ostream & operator<<( std::ostream & s, const QByteArray & str ) {
	return s << str.constData();
    }
    static inline std::ostream & operator<<( std::ostream & s, const QString & str ) {
	return s << str.toUtf8().constData();
    }
    static inline std::ostream & operator<<( std::ostream & s, const QVariant & v ) {
	return s << v.toString();
    }
QT_END_NAMESPACE

KDAB_UNITTEST_SIMPLE( KDSignalSpy, "kdtools/core" ) {

    KDSignalSpy spy;

    QObject * o_ptr = 0;
    {
	QObject o;
	KDAB_SET_OBJECT_NAME( o );
	o_ptr = &o;
	spy.addObject( &o );
    }

    assertEqual( spy.events().size(), 2 );
    const QVector<KDSignalSpy::Event> ev = spy.events();

    assertEqual( ev[0].object, o_ptr );
    assertEqual( ev[1].object, o_ptr );
    assertEqual( ev[0].signal, QByteArray("QObject::destroyed(QObject*)") );
    assertEqual( ev[1].signal, QByteArray("QObject::destroyed()") );
    assertEqual( ev[0].arguments.size(), 1 );
    assertEqual( ev[1].arguments.size(), 0 );
    assertEqual( qVariantValue<QObject*>( ev[0].arguments[0] ), o_ptr );
}

#endif // KDTOOLSCORE_UNITTESTS
