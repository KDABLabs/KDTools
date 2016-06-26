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

#include "kdlog.h"
#include "kdlog_p.h"

#include <QCoreApplication>
#include <QFile>
#include <QTextCodec>
#include <QByteArray>
#include <QString>
#include <QVector>

#include <cassert>

/*!
  \defgroup kdlog KDLog
  \ingroup core
*/

//
// KDLogDevice
//

/*!
  \class KDLogDevice
  \ingroup kdlog
  \brief The abstract base class for log devices.

  Abstract base class for log devices. Subclass this and implement the
  virtual method \ref log( int severity, const char* fmt, va_list ap )

  Subclasses can be used with the \ref KDLog class.

  \note At present, the classes of the \link kdlog KDLog
  Module\endlink are \em not safe to use in another than the GUI thread.
*/

/*!
  \fn void KDLogDevice::log( KDLog::Severity severity, const QString & msg )

  Log \a msg with severity \a severity.
*/


//
// KDLog
//

static KDLog * qt_msg_logger = 0;

static void kdlogger_message_handler( QtMsgType type, const char * msg ) {
    if ( !qt_msg_logger ) return;
    switch ( type ) {
    case QtDebugMsg:
        qt_msg_logger->logDebug( "%s", msg );
        break;
    case QtWarningMsg:
        qt_msg_logger->logWarning( "%s", msg );
        break;
    case QtCriticalMsg: // ### make a different level for this?
    case QtFatalMsg:
        qt_msg_logger->logError( "%s", msg );
        break;
    }
}

/*!
  \class KDLog
  \ingroup kdlog
  \brief A class for logging messages to a \ref KDLogDevice.

  \note At present, the classes of the \link kdlog KDLog
  Module\endlink are \em not safe to use in another than the GUI thread.
*/

class KDLog::Private {
    friend class ::KDLog;
public:
    Private( KDLogDevice * ld ) : logDevice( ld ) {}
    ~Private() {}

private:
    KDLogDevice * logDevice;
};

/*!
  \enum KDLog::Severity

  Specifies the urgency of the log message. Info is least urgent,
  Error most urgent.
*/

/*!
  Constructor. You normally create one instance of this during
  application initialization and use it throughout the life of the
  application.
  \param logDev A pointer to a \ref KDLogDevice that is used for
  logging. The \ref KDLogDevice must be created on the heap, and the
  KDLog takes ownership of the log device.
 */
KDLog::KDLog( KDLogDevice * logDev )
    : d( new Private( logDev ) )
{
    if ( !logDev )
        qWarning( "KDLog: NULL KDLogDevice" );
}

static void install_qt_msg_logger( KDLog * logger ) {
    assert( logger != 0 );
    qt_msg_logger = logger;
    qInstallMsgHandler( kdlogger_message_handler );
}

static void uninstall_qt_msg_logger() {
    assert( qt_msg_logger != 0 );
    qInstallMsgHandler( 0 );
    qt_msg_logger = 0;
}

/*!
  \property bool KDLog::qDebugMessagesRedirected

  Specifies whether this log instance receives QDebug messages (\a on
  == \c true) or not (\a on == \c false). The default is \c false.

  \note Only one KDLog instance at a time can receive QDebug
  messages. If you have set another KDLog instance to receive QDebug
  messages, calling this function with \a on == \c true will \em steal
  the message redirection from that instance and install itself to
  receive QDebug messages. If, however, called with \a on == \c false,
  this function does nothing.

  Get this property's value using %qDebugMessagesRedirected() and set
  it using %setQDebugMessagesRedirected().
*/
void KDLog::setQDebugMessagesRedirected( bool on ) {
    if ( on ) {
        if ( qt_msg_logger ) {
            if ( qt_msg_logger != this )
                qWarning( "KDLog::setQDebugMessagesRedirected: "
                          "Stealing Qt message handler ownership from KDLog instance %p.", static_cast<void*>(qt_msg_logger) );
            else
                qWarning( "KDLog::setQDebugMessagesRedirected: "
                          "already Qt message handler." );
        }
        install_qt_msg_logger( this );
    } else {
        if ( qt_msg_logger == this )
            uninstall_qt_msg_logger();
        else
            qWarning( "KDLog::setQDebugMessagesRedirected: "
                      "not current Qt message handler! Not resetting the handler." );
    }
}

bool KDLog::qDebugMessagesRedirected() const {
    return qt_msg_logger == this;
}

/*!
  Destructor. Deletes the \ref KDLogDevice associated with this
  KDLog and deregisters itself as a handler for qDebug(),
  qWarning() and qFatal() if it was installed as a Qt message handler.
*/
KDLog::~KDLog() {
    if ( qt_msg_logger == this )
        uninstall_qt_msg_logger();
    delete d->logDevice; d->logDevice = 0;
}

/*!
  Write an informational entry to the log.
  \param fmt A printf-style format string.
 */
void KDLog::logInfo( const char * fmt, ... ) {
    if ( !d->logDevice )
        return;

    va_list ap;
    va_start( ap, fmt );
    const QString str = QString().vsprintf( fmt, ap );
    va_end( ap );

    d->logDevice->log( Info, str );
}

/*!
  Write a debugging-related entry to the log.
  \param fmt A printf-style format string.
 */
void KDLog::logDebug( const char * fmt, ... ) {
    if ( !d->logDevice )
        return;

    va_list ap;
    va_start( ap, fmt );
    const QString str = QString().vsprintf( fmt, ap );
    va_end( ap );

    d->logDevice->log( Debug, str );
}

/*!
  Write a warning to the log.
  \param fmt A printf-style format string.
 */
void KDLog::logWarning( const char * fmt, ... ) {
    if ( !d->logDevice )
        return;

    va_list ap;
    va_start( ap, fmt );
    const QString str = QString().vsprintf( fmt, ap );
    va_end( ap );

    d->logDevice->log( Warning, str );
}

/*!
  Write an error-message to the log.
  \param fmt A printf-style format string.
 */
void KDLog::logError( const char * fmt, ... ) {
    if ( !d->logDevice )
        return;

    va_list ap;
    va_start( ap, fmt );
    const QString str = QString().vsprintf( fmt, ap );
    va_end(ap);

    d->logDevice->log( Error, str );
}

//
// KDLogDevice
//

#define d d_func()

KDLogDevice::KDLogDevice() : _d( 0 ) { init( false ); }

KDLogDevice::~KDLogDevice() {}

void KDLogDevice::init( bool ) {}

//
// KDEncodingLogDevice
//

/*!
  \class KDEncodingLogDevice
  \ingroup kdlog
  \brief Helper class for KDLogDevice implementations that need to encode Unicode

  This class is intended as an implementation aid for KDLogDevice. It
  implements KDLogDevice::log() by encoding the string using the
  textCodec() set on it, and handing off the result to doLogEncoded()
  for processing by further subclasses.

  All functions in this class are strongly exception-safe.
*/

/*!
  Create a KDEncodingLogDevice using QTextCodec::codecForLocale() to
  encode Unicode.
*/
KDEncodingLogDevice::KDEncodingLogDevice()
    : KDLogDevice( new Private( QTextCodec::codecForLocale() ), false )
{
    init( false );
}

/*!
  Create a KDEncodingLogDevice using \a codec to encode Unicode.
*/
KDEncodingLogDevice::KDEncodingLogDevice( const QTextCodec * codec )
    : KDLogDevice( new Private( codec ), false )
{
    init( false );
}

/*!
  Destructor
*/
KDEncodingLogDevice::~KDEncodingLogDevice() {}

void KDEncodingLogDevice::init( bool ) {}

/*!
  Set \a codec as the codec to use for encoding unicode. If \a
  codec is 0, log() will use QString::toLocal8Bit() for the
  conversion.

  \sa textCodec()
*/
void KDEncodingLogDevice::setTextCodec( const QTextCodec * codec ) {
    d->codec = codec;
}

const QTextCodec * KDEncodingLogDevice::textCodec() const {
    return d->codec;
}

/*!
  \fn const QTextCodec * KDEncodingLogDevice::textCodec() const

  Returns the codec used for encoding Unicode.

  \sa setTextCodec()
*/

/*!
  Subclasses of KDEncodingLogDevice should implemented doLogEncoded()
  instead.
*/
void KDEncodingLogDevice::log( KDLog::Severity severity, const QString & msg ) {
    doLogEncoded( severity, d->codec ? d->codec->fromUnicode( msg ) : msg.toLocal8Bit() );
}

/*!
  \fn void doLogEncoded( KDLog::Severity severity, const QByteArray & msg )

  Called by this log() implementation after recoding the message from
  Unicode into some charset given by textCodec().
*/

//
// KDStderrLogDevice
//

/*!
  \class KDStderrLogDevice
  \ingroup kdlog
  \brief A KDLogDevice that writes its output to stderr
*/

/*!
  The constructor. Constructs a KDStderrLogDevice object.
*/
KDStderrLogDevice::KDStderrLogDevice() : KDEncodingLogDevice() {}
KDStderrLogDevice::KDStderrLogDevice( const QTextCodec * codec ) : KDEncodingLogDevice( codec ) {}
KDStderrLogDevice::~KDStderrLogDevice() {}

void KDStderrLogDevice::init( bool ) {}

static const char * severity_to_string( KDLog::Severity severity ) {
    switch( severity ) {
    case KDLog::Info:
        return "Info: ";
    case KDLog::Debug:
        return "Debug: ";
    case KDLog::Warning:
        return "Warning: ";
    case KDLog::Error:
        return "Error: ";
    }
    return "";
}

/*!
  Reimplemented from \ref KDLogDevice
*/
void KDStderrLogDevice::doLogEncoded( KDLog::Severity severity, const QByteArray & msg ) {
    fprintf( stderr, "%s%s\n", severity_to_string( severity ), msg.data() ? msg.data() : "" );
}

//
// KDCompositeLogDevice
//

/*!
  \class KDCompositeLogDevice
  \ingroup kdlog
  \brief A KDLogDevice that enables output to multiple log devices.
*/

class KDCompositeLogDevice::Private : public KDLogDevice::Private {
    friend class ::KDCompositeLogDevice;
public:

private:
    QVector<KDLogDevice*> logDevices;
};

/*!
  Constructor. Creates a new composite log device.
*/
KDCompositeLogDevice::KDCompositeLogDevice()
    : KDLogDevice( new Private, false )
{
    init( false );
}


/*!
  Destructor.
*/
KDCompositeLogDevice::~KDCompositeLogDevice() {
    qDeleteAll( d->logDevices );
}

void KDCompositeLogDevice::init( bool ) {}

/*!

  Add KDLogDevice \a dev to this KDCompositeLogDevice. If several log
  devices are added after each other, they are called in the order
  they were added.

  \note The KDLogDevice must be allocated on the heap, as
  KDCompositeLogDevice takes overship of it.
*/
void KDCompositeLogDevice::addLogDevice( KDLogDevice * dev ) {
    if ( dev )
        d->logDevices.push_back( dev );
}

/*!
  Reimplemented from \ref KDLogDevice
*/
void KDCompositeLogDevice::log( KDLog::Severity severity, const QString & msg ) {
    Q_FOREACH( KDLogDevice * dev, d->logDevices )
        dev->log( severity, msg );
}


//
// KDFileLogDevice
//


/*!
  \class KDFileLogDevice
  \ingroup kdlog
  \brief A KDLogDevice that outputs to a file.
*/

class KDFileLogDevice::Private : public KDEncodingLogDevice::Private {
    friend class ::KDFileLogDevice;
public:
    explicit Private( const QString & filename, QIODevice::OpenMode mode, const QTextCodec * c )
        : KDEncodingLogDevice::Private( c ), file( filename )
    {
        mode &= QIODevice::Append|QIODevice::Truncate|QIODevice::Text;
        mode |= QIODevice::WriteOnly;

        if ( !file.open( mode ) )
            fprintf( stderr, "KDFileLogDevice: Unable to open logfile %s: %s\n",
                     qPrintable( file.fileName() ), qPrintable( file.errorString() ) );
    }

public:
    QFile file;
};

/*!
  Constructor.
  \param filename The name of the file that the log is written to
  \param deleteFile If true, the file is deleted before writing the
  first logentry. The default is to append to the file if it already
  exists.
 */
KDFileLogDevice::KDFileLogDevice( const QString& fileName, QIODevice::OpenMode mode )
    : KDEncodingLogDevice( new Private( fileName, mode, QTextCodec::codecForLocale() ), false )
{
    init( false );
}

KDFileLogDevice::KDFileLogDevice( const QString & fileName, const QTextCodec * codec, QIODevice::OpenMode mode )
    : KDEncodingLogDevice( new Private( fileName, mode, codec ), false )
{
    init( false );
}

void KDFileLogDevice::init( bool ) {}

/*!
  Destructor. Closes the associated file.
*/
KDFileLogDevice::~KDFileLogDevice() {}

/*!
  Reimplemented from \ref KDLogDevice
 */
void KDFileLogDevice::doLogEncoded( KDLog::Severity severity, const QByteArray & msg ) {
    if( !d->file.isOpen() )
        return;

    const char * const severityString = severity_to_string( severity );
    // ### error-handle write's...
    d->file.write( severityString, qstrlen( severityString ) );
    d->file.write( msg );
    d->file.write( "\n", 1 );
}

/*!
  \class KDSignalLogDevice
  \ingroup kdlog
  \brief A KDLogDevice that emits signals when a log entry is written to it
*/

/*!
  The constructor. Constructs a KDSignalLogDevice object.
*/
KDSignalLogDevice::KDSignalLogDevice() : QObject( 0 ) {}

/*!
  Destructor
*/
KDSignalLogDevice::~KDSignalLogDevice() {}

void KDSignalLogDevice::init( bool ) {}

/*!
  Reimplemented from \ref KDLogDevice
 */
void KDSignalLogDevice::log( KDLog::Severity severity, const QString & msg ) {
    switch ( severity ) {
    case KDLog::Info:
        emit info( msg );
        break;
    case KDLog::Debug:
        emit debug( msg );
        break;
    case KDLog::Warning:
        emit warning( msg );
        break;
    case KDLog::Error:
        emit error( msg );
        break;
    }
}


/*!
  \fn void KDSignalLogDevice::info( const QString & msg )

  This signal is emitted when a log message at severity Info is sent
  to this device.
*/


/*!
  \fn void KDSignalLogDevice::debug( const QString & msg )

  This signal is emitted when a log message at severity Debug is sent
  to this device.
*/


/*!
  \fn void KDSignalLogDevice::warning( const QString & msg )

  This signal is emitted when a log message at severity Warning is sent
  to this device.
*/


/*!
  \fn void KDSignalLogDevice::error( const QString & msg )

  This signal is emitted when a log message at severity Error is sent
  to this device.
*/

//
// KDSystemLogDevice
//

/*!
  \class KDSystemLogDevice
  \ingroup kdlog
  \brief A KDLogDevice that write to the system log. On UNIX this means
  writing to the syslog and on Windows it means logging to the
  Application eventlog.
*/

/*!
  \enum KDSystemLogDevice::Facility

  Syslog facilities User, Damon and Auth.
*/

/*!
  \fn KDSystemLogDevice::KDSystemLogDevice( Facility facility )

  Constructor.
  \param facility The syslog facility to use. The values
  enumerated in \ref KDSystemLogDevice::Facility are supported on
  UNIX. On windows this parameter currently does nothing.
*/

/*!
  \fn KDSystemLogDevice::~KDSystemLogDevice()

  Closes the connection to the system log.
*/

/*!
  \fn void KDSystemLogDevice::log( KDLog::Severity severity, const QString & msg )

  Reimplemented from \ref KDLogDevice.
*/

#undef d

#include "moc_kdlog.cpp"
