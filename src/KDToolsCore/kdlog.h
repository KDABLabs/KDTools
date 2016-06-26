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

#ifndef __KDTOOLSCORE_KDLOG_H__
#define __KDTOOLSCORE_KDLOG_H__

#include <KDToolsCore/kdtoolsglobal.h>
#include <KDToolsCore/pimpl_ptr.h>

#include <QtCore/QObject>
#include <QtCore/QIODevice>

QT_BEGIN_NAMESPACE
class QString;
class QTextCodec;
class QByteArray;
QT_END_NAMESPACE

class KDLogDevice;

class KDTOOLSCORE_EXPORT KDLog {
    Q_DISABLE_COPY( KDLog )
    DOXYGEN_PROPERTY( bool qDebugMessagesRedirected READ qDebugMessagesRedirected WRITE setQDebugMessagesRedirected )
public:
    explicit KDLog( KDLogDevice * logDev );
    ~KDLog();

    enum _Severity {
        Info    = 0x1,
        Debug   = 0x2,
        Warning = 0x3,
        Error   = 0x4,
        Popup   = 0x100,
        User    = 0x200,
        LevelMask = Info|Debug|Warning|Error
    };
    Q_DECLARE_FLAGS( Severity, _Severity )

    void setQDebugMessagesRedirected( bool on );
    bool qDebugMessagesRedirected() const;

    void logInfo( const char* fmt, ... )
#if defined(Q_CC_GNU) && !defined(__INSURE__)
    __attribute__ ((format (printf, 2, 3)))
#endif
        ;
    void logDebug( const char* fmt, ... )
#if defined(Q_CC_GNU) && !defined(__INSURE__)
    __attribute__ ((format (printf, 2, 3)))
#endif
        ;
    void logWarning( const char* fmt, ... )
#if defined(Q_CC_GNU) && !defined(__INSURE__)
    __attribute__ ((format (printf, 2, 3)))
#endif
        ;
    void logError( const char* fmt, ... )
#if defined(Q_CC_GNU) && !defined(__INSURE__)
    __attribute__ ((format (printf, 2, 3)))
#endif
        ;

private:
    class Private;
    kdtools::pimpl_ptr<Private> d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( KDLog::Severity )

class KDTOOLSCORE_EXPORT KDLogDevice {
    Q_DISABLE_COPY( KDLogDevice )
public:
    KDLogDevice();
    virtual ~KDLogDevice();

    virtual void log( KDLog::Severity severity, const QString & msg ) = 0;

private:
    KDTOOLS_DECLARE_PRIVATE_BASE( KDLogDevice );
};

class KDTOOLSCORE_EXPORT KDEncodingLogDevice : public KDLogDevice {
public:
    KDEncodingLogDevice();
    explicit KDEncodingLogDevice( const QTextCodec * codec );
    ~KDEncodingLogDevice();

    void setTextCodec( const QTextCodec * codec );
    const QTextCodec * textCodec() const;

    void log( KDLog::Severity severity, const QString & msg ) KDAB_OVERRIDE;

private:
    virtual void doLogEncoded( KDLog::Severity severity, const QByteArray & msg ) = 0;

private:
    KDTOOLS_DECLARE_PRIVATE_DERIVED( KDEncodingLogDevice, KDLogDevice );
};

class KDTOOLSCORE_EXPORT KDStderrLogDevice : public KDEncodingLogDevice {
public:
    KDStderrLogDevice();
    explicit KDStderrLogDevice( const QTextCodec * codec );
    ~KDStderrLogDevice();

private:
    void doLogEncoded( KDLog::Severity severity, const QByteArray & msg ) KDAB_OVERRIDE;
private:
    KDTOOLS_DECLARE_PRIVATE_DERIVED( KDStderrLogDevice, KDEncodingLogDevice );
};

class KDTOOLSCORE_EXPORT KDFileLogDevice : public KDEncodingLogDevice {
public:
    explicit KDFileLogDevice( const QString & filename, QIODevice::OpenMode mode=QIODevice::Append );
    explicit KDFileLogDevice( const QString & filename, const QTextCodec * codec, QIODevice::OpenMode mode=QIODevice::Append );
    ~KDFileLogDevice();

private:
    void doLogEncoded( KDLog::Severity severity, const QByteArray & msg );

private:
    KDTOOLS_DECLARE_PRIVATE_DERIVED( KDFileLogDevice, KDEncodingLogDevice );
};

class KDTOOLSCORE_EXPORT KDSignalLogDevice : public QObject, public KDLogDevice {
    Q_OBJECT
public:
    KDSignalLogDevice();
    ~KDSignalLogDevice();

    void log( KDLog::Severity severity, const QString & msg ) KDAB_OVERRIDE;

Q_SIGNALS:
    void info( const QString & msg );
    void debug( const QString & msg );
    void warning( const QString & msg );
    void error( const QString & msg );

private:
    KDTOOLS_DECLARE_PRIVATE_DERIVED( KDSignalLogDevice, KDLogDevice );
};

class KDTOOLSCORE_EXPORT KDSystemLogDevice : public KDEncodingLogDevice {
public:
    enum Facility { User, Daemon, Auth };

    explicit KDSystemLogDevice( Facility facility );
    explicit KDSystemLogDevice( Facility facility, const QTextCodec * codec );
    ~KDSystemLogDevice();

private:
    void doLogEncoded( KDLog::Severity severity, const QByteArray & msg ) KDAB_OVERRIDE;
private:
    KDTOOLS_DECLARE_PRIVATE_DERIVED( KDSystemLogDevice, KDEncodingLogDevice );
    Facility mFacility;
    void * mInternal;
};

class KDTOOLSCORE_EXPORT KDCompositeLogDevice : public KDLogDevice {
public:
    KDCompositeLogDevice();
    ~KDCompositeLogDevice();

    void addLogDevice( KDLogDevice * ld );

    void log( KDLog::Severity severity, const QString & msg ) KDAB_OVERRIDE;

private:
    KDTOOLS_DECLARE_PRIVATE_DERIVED( KDCompositeLogDevice, KDLogDevice );
};

#endif /* __KDTOOLSCORE_KDLOG_H__ */
