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

#include "kdupdaterufcompressor.h"
#include "kdupdaterufcompresscommon_p.h"

#include <QCryptographicHash>
#include <QtDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QPointer>

#include <cassert>

struct KDUpdater::UFCompressor::UFCompressorData
{
    UFCompressorData( UFCompressor* qq ) :
        q( qq )
    {}

    UFCompressor* q;

    QString ufFileName;
    QString source;
    QString errorString;
    
    static void updateUFHeader(const QString& stripFromPath, const QDir& dir, KDUpdater::UFHeader& header);
    static QString fileNameRelativeTo(const QString& fileName, const QString& relativeTo);
    void setError( const QString& msg );
};

void KDUpdater::UFCompressor::UFCompressorData::setError( const QString& msg )
{
    errorString = msg;
}

KDUpdater::UFCompressor::UFCompressor()
    : d ( new UFCompressorData( this ) )
{
}

KDUpdater::UFCompressor::~UFCompressor()
{
    delete d;
}

QString KDUpdater::UFCompressor::errorString() const
{
    return d->errorString;
}

void KDUpdater::UFCompressor::setFileName(const QString& fileName)
{
    d->ufFileName = fileName;
}

QString KDUpdater::UFCompressor::fileName() const
{
    return d->ufFileName;
}

void KDUpdater::UFCompressor::setSource(const QString& source)
{
    d->source = source;
}

QString KDUpdater::UFCompressor::source() const
{
    return d->source;
}

namespace {
    class FileRemover {
    public:
        explicit FileRemover( QFile* f ) : file( f )
        {
            assert( file );
        }

        ~FileRemover()
        {
            if ( !file )
                return;
            file->close();
            if ( !file->exists() )
                return;
            const bool deleted = file->remove();
            if ( !deleted )
                qWarning("Could not delete incomplete file %s: %s", qPrintable(file->fileName()), qPrintable(file->errorString()));
        }

        void finalizeAndRelease()
        {
            if ( !file )
                return;
            file->close();
            file = 0;
        }

    private:
        QPointer<QFile> file;
    };
}

bool KDUpdater::UFCompressor::compress()
{
    d->errorString.clear();
   
    // Perform some basic checks.
    QFileInfo sourceInfo(d->source);
    if( !sourceInfo.isReadable() ) {
        d->setError( tr( "\"%1\" is not readable").arg( d->source ) );
        return false;
    }

    QDir sourceDir( sourceInfo.absoluteFilePath() ); // = sourceInfo.dir();
    sourceDir.cdUp();
    QString sourcePath = sourceDir.absolutePath();

    // First create the ZIP header.
    KDUpdater::UFHeader header;
    header.magic = QLatin1String( KD_UPDATER_UF_HEADER_MAGIC );
    header.fileList << d->fileNameRelativeTo(sourceInfo.absoluteFilePath(), sourcePath);
    header.permList << static_cast<quint64>(sourceInfo.permissions());
    header.isDirList << sourceInfo.isDir();
    // qDebug("ToCompress: %s", qPrintable(header.FileList.first()));

    if(sourceInfo.isDir())
        d->updateUFHeader(sourcePath, QDir(d->source), header);

    // open the uf file for writing
    QFile ufFile( d->ufFileName );
    //this should actually use temp files for security, for now remove partial files if saving failed  
    FileRemover remover( &ufFile );
    
    if( !ufFile.open(QFile::WriteOnly) )
    {
        d->setError( tr( "Could not open \"%1\" for writing: %2").arg( d->ufFileName, ufFile.errorString() ) );
        return false;
    }
    
    QDataStream ufDS( &ufFile );
    ufDS.setVersion( QDataStream::Qt_4_2 );
    QCryptographicHash hash( QCryptographicHash::Md5 );

    // Insert the header into the UF file
    ufDS << header;
    header.addToHash(hash);

    // Now create ZIP entries and add them.
    for(int i=0; i<header.fileList.count(); i++)
    {
        if(header.isDirList[i])
            continue;

        KDUpdater::UFEntry ufEntry;
        ufEntry.fileName = header.fileList[i];

        QString completeFileName = QString::fromLatin1( "%1/%2" ).arg(sourcePath, ufEntry.fileName);
        QFile zeFile( completeFileName );
        if ( !zeFile.open( QFile::ReadOnly ) ) {
            d->setError( tr( "Could not open input file \"%1\" to compress: %2").arg( completeFileName, zeFile.errorString() ) );
            return false;
        }
        ufEntry.fileData = qCompress(zeFile.readAll());
        ufEntry.permissions = static_cast<quint64>(zeFile.permissions());

        //qDebug("Compressed %s as %s", qPrintable(completeFileName), qPrintable(ufEntry.fileName));

        ufDS << ufEntry;
        ufEntry.addToHash(hash);
    }

    // All done, append hash and close file
    ufDS << hash.result();
    ufFile.close();
    
    if ( ufFile.error() != QFile::NoError )
    {
        d->setError( tr( "Could not save compressed data to \"%1\": %2").arg( ufFile.fileName(), ufFile.errorString() ) );
        return false;
    }

    remover.finalizeAndRelease(); // do not remove the file

    return true;
}

void KDUpdater::UFCompressor::UFCompressorData::updateUFHeader(const QString& stripFromPath, const QDir& dir, KDUpdater::UFHeader& header)
{
    // qDebug() << dir.absolutePath();
    QFileInfoList children = dir.entryInfoList(QDir::AllEntries|QDir::NoDotAndDotDot|QDir::NoSymLinks);
	Q_FOREACH(const QFileInfo& fi, children)
    {
        // Skip files that are not readable.
        if( !fi.isReadable() )
            continue;

        // Format the file name relative to stripFromPath
        QString fileName = fileNameRelativeTo(fi.absoluteFilePath(), stripFromPath);
        header.fileList << fileName;
        header.permList << static_cast<quint64>(fi.permissions());
        header.isDirList << fi.isDir();
        // qDebug("ToCompress: %s", qPrintable(fileName));

        if(fi.isDir())
        {
            QDir dir2 = dir;
            dir2.cd(fi.fileName());
            updateUFHeader(stripFromPath, dir2, header);
            // dir.cdUp();
        }
    }
}

QString KDUpdater::UFCompressor::UFCompressorData::fileNameRelativeTo(const QString& fileName, const QString& relativeTo)
{
    const QDir dir( relativeTo );
    const QString ret = dir.relativeFilePath( fileName );
    return QDir::fromNativeSeparators(ret);

    /*
      QString ret = fileName;
      int len = relativeTo.length();
      if(ret.startsWith(relativeTo))
      ret.remove(0, len);
      if(ret.startsWith('/') || ret.startsWith('\\'))
      ret.remove(0, 1);
      if(ret.endsWith('/') || ret.endsWith('\\'))
      ret.remove(ret.length()-1, 1);
      ret = ret.replace('\\', '/');
      return ret;
    */
}
