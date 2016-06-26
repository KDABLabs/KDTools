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

#ifndef __KDTOOLSCORE_KDSAVEFILE_H__
#define __KDTOOLSCORE_KDSAVEFILE_H__

#include <KDToolsCore/pimpl_ptr.h>

#include <QtCore/QFile>

class KDTOOLSCORE_EXPORT KDSaveFile : public QIODevice
{
    Q_OBJECT
public:
    explicit KDSaveFile( QObject* parent = 0 );
    explicit KDSaveFile( const QString& filename, QObject* parent = 0 );
    ~KDSaveFile();

    enum CommitMode {
        BackupExistingFile=0x1,
        OverwriteExistingFile=0x2
    };

    bool commit( CommitMode=BackupExistingFile );

    QFile::FileError error() const;
    void unsetError();

    QString fileName() const;
    void setFileName( const QString& filename );

    QFile::Permissions permissions() const;
    bool setPermissions( QFile::Permissions );

    QString backupExtension() const;
    void setBackupExtension( const QString& extension );

    bool flush();
    bool resize( qint64 sz );
    int handle() const;

    qint64 bytesAvailable() const;
    qint64 bytesToWrite() const;
    bool canReadLine() const;
    void close();
    bool isSequential() const;
    bool open( OpenMode mode=QIODevice::ReadWrite ); //only valid: WriteOnly, ReadWrite
    bool seek( qint64 pos );
    qint64 size() const;
    bool waitForBytesWritten( int msecs );
    bool waitForReadyRead( int msecs );

protected:
    qint64 readData( char* data, qint64 maxSize );
    qint64 readLineData( char* data, qint64 maxSize );
    qint64 writeData( const char* data, qint64 maxSize );

private:
    class Private;
    kdtools::pimpl_ptr<Private> d;
};

#endif // __KDTOOLSCORE_KDSAVEFILE_H__
