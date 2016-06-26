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

#include <QByteArray>
#include <QTextCodec>

#include <stdio.h>
#include <stdlib.h>

#include <windows.h>
#include <winbase.h>

#include <cassert>

class KDSystemLogDevice::Private : public KDEncodingLogDevice::Private {
    friend class ::KDSystemLogDevice;
public:
    explicit  Private( Facility facility_ )
        : KDEncodingLogDevice::Private( QTextCodec::codecForLocale() ),
          facility( facility_ ),
          internal( RegisterEventSource( 0, TEXT("Application") ) )
    {
        if ( !internal )
            fprintf( stderr, "Error in RegisterEventSourceA(): %u\n", static_cast< unsigned int >( GetLastError() ) );
    }
private:
    Facility facility;
    HANDLE internal;
};

KDSystemLogDevice::KDSystemLogDevice( Facility facility )
    : KDEncodingLogDevice( new Private( facility ), false )
{
    init( false );
}

#define d d_func()

KDSystemLogDevice::~KDSystemLogDevice() {
    DeregisterEventSource( d->internal );
}

void KDSystemLogDevice::init( bool ) {}

static int severityToEventLogType( KDLog::Severity severity ) {
    switch ( severity & KDLog::LevelMask ) {
    case KDLog::Info:
    case KDLog::Debug: // No debug option
        return EVENTLOG_INFORMATION_TYPE;
    case KDLog::Warning:
        return EVENTLOG_WARNING_TYPE;
    case KDLog::Error:
        return EVENTLOG_ERROR_TYPE;
    }
    assert( !"KDSystemLogDevice::log: unknown Severity value encountered" );
    return EVENTLOG_INFORMATION_TYPE;
}

void KDSystemLogDevice::doLogEncoded( KDLog::Severity severity, const QByteArray & msg ) {
    const char* strarray[1];
    strarray[0] = msg.constData();
    if ( ReportEventA( d->internal,
                       severityToEventLogType( severity ),
                       0, // category?
                       0, // Event ID
                       NULL, // user security identifier (optional)
                       1, // number of strings to merge with message
                       0, // size of raw (binary) data (in bytes)
                       strarray, // array of strings to merge with message
                       NULL ) )
        return;

    // error:

    char* lpMsgBuf;
    const int err = GetLastError();
    FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER|
                    FORMAT_MESSAGE_FROM_SYSTEM|
                    FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL,
                    err,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT ),
                    (LPSTR)(&lpMsgBuf),
                    0,
                    NULL);
    fprintf( stderr, "ReportEventA() failed: %u=\"%s\" Message was \"%s\"\n",
             err, lpMsgBuf, msg.constData() );
}

#undef d
