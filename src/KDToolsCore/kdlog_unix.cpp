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
#include <QByteArray>
#include <QStringList>
#include <QTextCodec>

#include <syslog.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>

static QByteArray findSyslogIdentification() {
    // ### check QCoreApplication::applicationName() etc
    return QCoreApplication::arguments().at( 0 ).toLocal8Bit();
}

static int toSyslogFacility( KDSystemLogDevice::Facility f ) {
    switch( f ) {
    case KDSystemLogDevice::User:   return LOG_USER;
    case KDSystemLogDevice::Daemon: return LOG_DAEMON;
    case KDSystemLogDevice::Auth:   return LOG_AUTH;
    }
    assert( !"Unknown KDSystemLogDevice::Facility encountered." );
    return 0;
}

static inline char * mystrdup( const char * str ) {
    using namespace std;
    return strdup( str );
}

class KDSystemLogDevice::Private : public KDEncodingLogDevice::Private {
    friend class ::KDSystemLogDevice;
public:
    explicit  Private( Facility facility_ )
        : KDEncodingLogDevice::Private( QTextCodec::codecForLocale() ),
          facility( facility_ ),
          // according to the openlog() docs, the identification string
          // better stay constant throughout the lifefile of the
          // connection, so therefore we take our own copy:
          internal( mystrdup( findSyslogIdentification().constData() ) )
    {
        assert( internal || !"Check why findSyslogIdentification() fails to return something" );
        openlog( internal, 0, toSyslogFacility( facility ) );
    }
private:
    Facility facility;
    char * internal;
};

KDSystemLogDevice::KDSystemLogDevice( Facility facility )
    : KDEncodingLogDevice( new Private( facility ), false )
{
    init( false );
}

KDSystemLogDevice::~KDSystemLogDevice() {
    closelog();
    using namespace std;
    if ( d_func()->internal )
        free( static_cast<void*>( d_func()->internal ) );
}

void KDSystemLogDevice::init( bool ) {}

static int severityToSyslogPriority( KDLog::Severity severity ) {
    switch( severity & KDLog::LevelMask ) {
    case KDLog::Info:    return LOG_INFO;
    case KDLog::Debug:   return LOG_DEBUG;
    case KDLog::Warning: return LOG_WARNING;
    case KDLog::Error:   return LOG_ERR;
    }
    assert( !"KDSystemLogDevice::log: unknown Severity value encountered" );
    return LOG_DEBUG;
}

void KDSystemLogDevice::doLogEncoded( KDLog::Severity severity, const QByteArray & msg ) {
    syslog( severityToSyslogPriority( severity ), "%s", msg.data() ? msg.data() : "" );
}

