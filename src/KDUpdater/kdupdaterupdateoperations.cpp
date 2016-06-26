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

#include "kdupdaterupdateoperations_p.h"
#include "kdupdatertarget.h"
#include "kdupdaterpackagesinfo.h"

#include <QFile>
#include <QDir>
#include <QDomDocument>
#include <QDirIterator>
#include <QProcess>
#include <QTemporaryFile>
#include <QTextCodec>
#include <QTextStream>
#include <QVariant>
#include <QString>
#include <cerrno>

#ifdef SUPPORT_DETACHED_PROCESS_EXECUTION
#ifdef Q_WS_WIN
#include <windows.h>
#endif
#endif

using namespace KDUpdater;

/**
 * Recursively removes the directory at \a path.
 */
static bool removeDirectory( const QString& path, QString* errorString = NULL )
{
    Q_ASSERT( errorString );
    const QFileInfoList entries = QDir( path ).entryInfoList( QDir::NoDotAndDotDot | QDir::AllEntries | QDir::Hidden );
    for( QFileInfoList::const_iterator it = entries.constBegin(); it != entries.constEnd(); ++it )
    {
        if( it->isDir() && !it->isSymLink() )
        {
            removeDirectory( it->filePath(), errorString );
        }
        else
        {
            QFile f( it->filePath() );
            if( !f.remove() )
                return false;
        }
    }

    errno = 0;
    const bool success = QDir().rmdir( path );
    if ( errno && errorString )
        *errorString = path + QLatin1String(": ") + QLatin1String( strerror(errno) );
    return success;
}
/*
 * \internal
 * Returns a filename for a temporary file based on \a templateName
 */
static QString backupFileName( const QString& templateName = QString() )
{
    QTemporaryFile file( templateName );
    file.open();
    const QString name = file.fileName();
    file.close();
    file.remove();
    return name;
}

////////////////////////////////////////////////////////////////////////////
// KDUpdater::CopyOperation
////////////////////////////////////////////////////////////////////////////

CopyOperation::CopyOperation()
{
    setName(QLatin1String( "Copy" ));
}

CopyOperation::~CopyOperation()
{
    QFile::remove( value( QLatin1String( "backupOfExistingDestination" ) ).toString() );
}

void CopyOperation::backup()
{
    const QString dest = arguments().last();
    if( !QFile::exists( dest ) )
    {
        clearValue( QLatin1String( "backupOfExistingDestination" ) );
        setError( UserDefinedError, tr("Nothing to backup") );
        return;
    }

    setValue( QLatin1String( "backupOfExistingDestination" ), backupFileName( dest ) );

    // race condition: The backup file could get created 
    // by another process right now. But this is the same 
    // in QFile::copy...
    const bool success = QFile::copy( dest, value( QLatin1String( "backupOfExistingDestination" ) ).toString() );
    if(!success)
        setError( UserDefinedError, tr("Could not backup file %1").arg(dest) );
}
// only a very basic implementation
void copyDirectory( const QString &src, const QString &dst )
{
    QDir srcDir( src );
    QDir destDir( dst );
    QFileInfoList devices = QDir::drives();
    QDir rootDir = QDir::root();
    Q_FOREACH( const QFileInfo &info, devices )
    {
        if ( destDir.absolutePath().contains( info.absoluteFilePath() ) )
            rootDir = QDir ( info.absoluteFilePath() );
    }
    if ( srcDir.exists() )
    {
        if ( !destDir.exists() )
            rootDir.mkpath( destDir.absolutePath() );
        QFileInfoList children = srcDir.entryInfoList( QStringList(), QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files );
        Q_FOREACH( const QFileInfo entry, children )
        {
            if ( !entry.isSymLink() )
            {
                if ( entry.isDir() )
                {
                    QDir nSrcDir = srcDir; nSrcDir.cd( entry.fileName() );
                    QDir nDestDir = destDir.absoluteFilePath( entry.fileName() );
                    copyDirectory( nSrcDir.absolutePath(), nDestDir.absolutePath() );
                }
                else if ( entry.isFile() )
                {
                    if ( QFile::exists( destDir.absoluteFilePath( entry.fileName() ) ) )
                        QFile::remove( destDir.absoluteFilePath( entry.fileName() ) );
                    QFile srcFile( entry.absoluteFilePath() );
                    srcFile.copy( destDir.absoluteFilePath( entry.fileName() ) );
                }
            }
            else
            {
                if ( QFile::exists( destDir.absoluteFilePath( entry.fileName() ) ) )
                    QFile::remove( destDir.absoluteFilePath( entry.fileName() ) );
                QFile::link( QFile::symLinkTarget( entry.absoluteFilePath() ), destDir.absoluteFilePath( entry.fileName() ) );
            }
        }
    }
}

bool CopyOperation::performOperation()
{
    // We need two args to complete the copy operation.
    // First arg provides the complete file name of source
    // Second arg provides the complete file name of dest
    QStringList args = this->arguments();
    if( args.count() != 2 )
    {
        setError( InvalidArguments, tr("Invalid arguments: %1 arguments given, 2 expected.").arg( args.count() ) );
        return false;
    }

    const QString& source = args.first();
    const QString& dest = args.last();
    const QFileInfo fi( source );

    if( !fi.isDir() )
    {
        // If destination file exists, then we cannot use QFile::copy()
        // because it does not overwrite an existing file. So we remove
        // the destination file.
        if( QFile::exists(dest) )
        {
            QFile destF( dest );
            if( !destF.remove() )
            {
                setError( UserDefinedError, tr("File %1 could not be deleted before copy: %2").arg(dest, destF.errorString()) );
                return false;
            }
        }

        // Perform the copy operation
        QFile sourceF( source );
        const bool success = sourceF.copy(dest);
        if(!success)
            setError( UserDefinedError, tr("Cannot copy file from %1 to %2: %3").arg(source, dest, sourceF.errorString()) );
        return success;
    }
    else
    {
        // If destination file exists, then we cannot use QFile::copy()
        // because it does not overwrite an existing file. So we remove
        // the destination file.
        // full version is still required
        copyDirectory( source, dest );
        return true;
    }
}

bool CopyOperation::undoOperation()
{
   const QString dest = arguments().last();

    QFile destF( dest );
    // first remove the dest
    if( !destF.remove() )
    {
        setError( UserDefinedError, tr("Could not delete file %1: %2").arg(dest, destF.errorString()) );
        return false;
    }

    // no backup was done:
    // the copy destination file wasn't existing yet - that's no error
    if( !hasValue( QLatin1String( "backupOfExistingDestination" ) ) )
        return true;

    QFile backupF( value( QLatin1String( "backupOfExistingDestination" ) ).toString() );
    // otherwise we have to copy the backup back:
    const bool success = backupF.copy( dest ) && backupF.remove();
    if(!success) {
        setError( UserDefinedError, tr("Could not restore backup file into %1: %2").arg(dest, backupF.errorString()) );
    }
    return success;
}

bool CopyOperation::testOperation()
{
    // TODO
    return true;
}

CopyOperation* CopyOperation::clone() const
{
    return UpdateOperation::clone< CopyOperation >();
}


////////////////////////////////////////////////////////////////////////////
// KDUpdater::MoveOperation
////////////////////////////////////////////////////////////////////////////

MoveOperation::MoveOperation()
{
    setName(QLatin1String( "Move" ));
}

MoveOperation::~MoveOperation()
{
    QFile::remove( value( QLatin1String( "backupOfExistingDestination" ) ).toString() ); 
}

void MoveOperation::backup()
{
    const QString dest = arguments().last();
    if( !QFile::exists( dest ) )
    {
        clearValue( QLatin1String( "backupOfExistingDestination" ) );
        setError( UserDefinedError, tr("Nothing to backup") );
        return;
    }

    setValue( QLatin1String( "backupOfExistingDestination" ), backupFileName( dest ) );

    // race condition: The backup file could get created 
    // by another process right now. But this is the same 
    // in QFile::copy...
    const bool success = QFile::copy( dest, value( QLatin1String( "backupOfExistingDestination" ) ).toString() );
    if(!success)
        setError( UserDefinedError, tr("Could not backup file %1").arg(dest) );
}

bool MoveOperation::performOperation()
{
    // We need two args to complete the copy operation.
    // First arg provides the complete file name of source
    // Second arg provides the complete file name of dest
    const QStringList args = this->arguments();
    if( args.count() != 2 )
    {
        setError( InvalidArguments, tr("Invalid arguments: %1 arguments given, 2 expected.").arg( args.count() ) );
        return false;
    }

    const QString& source = args.first();
    const QString& dest = args.last();

    // If destination file exists, then we cannot use QFile::copy()
    // because it does not overwrite an existing file. So we remove
    // the destination file.
    if( QFile::exists(dest) )
    {
        QFile destF( dest );
        if( !destF.remove(dest) )
        {
            setError( UserDefinedError, tr("Cannot remove file %1: %2").arg(dest, destF.errorString()) );
            return false;
        }
    }

    // Copy source to destination.
    QFile sourceF( source );
    const bool success = sourceF.copy( dest ) && sourceF.remove();
    if(!success)
        setError( UserDefinedError, tr("Cannot move file from %1 to %2: %3").arg( source, dest, sourceF.errorString() ) );

    return success;
}

bool MoveOperation::undoOperation()
{
    const QStringList args = arguments();
    const QString& source = args.first();
    const QString& dest = args.last();

    // first: copy back the destination to source
    QFile destF( dest );
    if( !destF.copy( source ) )
    {
        setError( UserDefinedError, tr("Cannot copy %1 to %2: %3").arg( dest, source, destF.errorString() ) );
        return false;
    }

    // no backup was done:
    // the move destination file wasn't existing yet - that's no error
    if( !hasValue( QLatin1String( "backupOfExistingDestination" ) ) )
        return true;

    // otherwise we have to copy the backup back:
    QFile backupF( value( QLatin1String( "backupOfExistingDestination" ) ).toString() );
    const  bool success = backupF.copy( dest ) && backupF.remove();
    if(!success)
        setError( UserDefinedError, tr("Cannot restore backup file for %1: %2").arg(dest, backupF.errorString()) );

    return success;
}

bool MoveOperation::testOperation()
{
    // TODO
    return true;
}

MoveOperation* MoveOperation::clone() const
{
    return UpdateOperation::clone< MoveOperation >();
}


////////////////////////////////////////////////////////////////////////////
// KDUpdater::DeleteOperation
////////////////////////////////////////////////////////////////////////////

DeleteOperation::DeleteOperation()
{
    setName(QLatin1String( "Delete" ));
}

DeleteOperation::~DeleteOperation()
{
    QFile::remove( value( QLatin1String( "backupOfExistingFile" ) ).toString() );
}

void DeleteOperation::backup()
{
    const QString fileName = arguments().first();
    setValue( QLatin1String( "backupOfExistingFile" ), backupFileName( fileName ) );
    QFile file( fileName );
    const bool success = file.copy( value( QLatin1String( "backupOfExistingFile" ) ).toString() );
    if(!success)
        setError( UserDefinedError, tr("Cannot create backup of %1: %2").arg(fileName, file.errorString()) );
}

bool DeleteOperation::performOperation()
{
    // Requires only one parameter. That is the name of
    // the file to remove.
    const QStringList args = this->arguments();
    if( args.count() != 1 )
    {
        setError( InvalidArguments, tr("Invalid arguments: %1 arguments given, 1 expected.").arg( args.count() ) );
        return false;
    }

    const QString& fName = args.first();
    QFile file( fName );
    const bool success = file.remove();
    if(!success)
        setError( UserDefinedError, tr("Cannot delete file %1: %2").arg(fName, file.errorString()) );

    return success;
}

bool DeleteOperation::undoOperation()
{
    if( !hasValue( QLatin1String( "backupOfExistingFile" ) ) )
    {
        setError( UserDefinedError, tr("Cannot find backup file to restore") );
        return false;
    }

    const QString fileName = arguments().first();
    QFile backupF( value( QLatin1String( "backupOfExistingFile" ) ).toString() );
    const bool success = backupF.copy( fileName ) && backupF.remove();
    if(!success)
        setError( UserDefinedError, tr("Cannot restore backup file for %1: %2").arg(fileName, backupF.errorString()) );

    return success;
}

bool DeleteOperation::testOperation()
{
    // TODO
    return true;
}

DeleteOperation* DeleteOperation::clone() const
{
    return UpdateOperation::clone< DeleteOperation >();
}


////////////////////////////////////////////////////////////////////////////
// KDUpdater::MkdirOperation
////////////////////////////////////////////////////////////////////////////

MkdirOperation::MkdirOperation()
{
    setName(QLatin1String( "Mkdir" ));
}

MkdirOperation::~MkdirOperation()
{

}

void MkdirOperation::backup()
{
    static const QRegExp re( QLatin1String( "\\\\|/" ) );
    static const QLatin1String sep( "/" );

    QString path = arguments().first();
    path.replace( re, sep );

    QDir createdDir = QDir::root();

    // find out, which part of the path is the first one we actually need to create
    int end = 0;
    while( true )
    {
        QString p = path.section( sep, 0, ++end );
        createdDir = QDir( p );
        if( !createdDir.exists() )
            break;
        else if( p == path )
        {
            // everything did already exist -> nothing to do for us (nothing to revert then, either)
            createdDir = QDir::root();
            break;
        }
    }

    setValue( QLatin1String( "createddir" ), createdDir.absolutePath() );
}

bool MkdirOperation::performOperation()
{
    // Requires only one parameter. That is the name of
    // the directory to create.
    const QStringList args = this->arguments();
    if( args.count() != 1 )
    {
        setError( InvalidArguments, tr("Invalid arguments: %1 arguments given, 1 expected.").arg( args.count() ) );
        return false;
    }

    const QString& dirName = args.first();
    const bool success = QDir::root().mkpath(dirName);
    if(!success)
        setError( UserDefinedError, tr("Could not create directory %1").arg(dirName) );

    return success;
}

bool MkdirOperation::undoOperation()
{
    const QDir createdDir = QDir( value( QLatin1String( "createddir" ) ).toString() );

    // no directory was created, see comment in backup()
    if( createdDir == QDir::root() )
        return true;

    const bool success = removeDirectory( createdDir.path() );
    if(!success)
        setError( UserDefinedError, tr("Could not remove directory %1").arg( createdDir.absolutePath() ) );

    return success;
}

bool MkdirOperation::testOperation()
{
    // TODO
    return true;
}

MkdirOperation* MkdirOperation::clone() const
{
    return UpdateOperation::clone< MkdirOperation >();
}

////////////////////////////////////////////////////////////////////////////
// KDUpdater::RmdirOperation
////////////////////////////////////////////////////////////////////////////

RmdirOperation::RmdirOperation()
{
    setValue( QLatin1String( "removed" ), false );
    setName(QLatin1String( "Rmdir" ));
}

RmdirOperation::~RmdirOperation()
{

}

void RmdirOperation::backup()
{
    // nothing to backup - rollback will just create the directory
}

bool RmdirOperation::performOperation()
{
    // Requires only one parameter. That is the name of
    // the file to remove.
    const QStringList args = this->arguments();
    if( args.count() != 1 )
    {
        setError( InvalidArguments, tr("Invalid arguments: %1 arguments given, 1 expected.").arg( args.count() ) );
        return false;
    }

    const QString dirName = args.first();
    const QDir dir( dirName );
    if( !dir.exists() )
    {
        setError( UserDefinedError, tr("Cannot delete non-existing directory %1").arg(dirName) );
        return false;
    }

    errno = 0;
    const bool removed = dir.rmdir( dirName );
    setValue( QLatin1String( "removed" ), removed );
    if ( !removed ) {
        setError( UserDefinedError );
        setErrorString( tr("Could not remove folder %1: %2.").arg( dirName, QLatin1String(strerror(errno)) ) );
    }
    return removed;
}

bool RmdirOperation::undoOperation()
{
   if( !value( QLatin1String( "removed" ) ).toBool() )
        return true;

    const QFileInfo fi( arguments().first() );
    const bool success = fi.dir().mkdir( fi.fileName() );
    if(!success)
        setError( UserDefinedError, tr("Cannot recreate directory %1: %2").arg( fi.fileName(), QLatin1String(strerror(errno)) ) );

    return success;
}

bool RmdirOperation::testOperation()
{
    // TODO
    return true;
}

RmdirOperation* RmdirOperation::clone() const
{
    return UpdateOperation::clone< RmdirOperation >();
}


////////////////////////////////////////////////////////////////////////////
// KDUpdater::AppendFileOperation
////////////////////////////////////////////////////////////////////////////

AppendFileOperation::AppendFileOperation()
{
    setName(QLatin1String( "AppendFile" ));
}

AppendFileOperation::~AppendFileOperation()
{

}

void AppendFileOperation::backup()
{
    const QString filename = arguments().first();

    QFile file( filename );
    if( !file.exists() )
        return; // nothing to backup

    setValue( QLatin1String( "backupOfFile" ), backupFileName( filename ) );
    if( !file.copy( value( QLatin1String( "backupOfFile" ) ).toString() ) )
    {
        setError( UserDefinedError, tr("Cannot backup file %1: %2").arg(filename, file.errorString()) );
        clearValue( QLatin1String( "backupOfFile" ) );
    }
}

bool AppendFileOperation::performOperation()
{
    // This operation takes two arguments. First argument is the name
    // of the file into which a text has to be appended. Second argument
    // is the text to append.
    // Optionally, a third parameter defines the text codec to be used
    const QStringList args = this->arguments();
    if( args.count() != 2 && args.count() != 3 )
    {
        setError( InvalidArguments, tr("Invalid arguments: %1 arguments given, 2 or 3 expected.").arg( args.count() ) );
        return false;
    }

    const QString& fName = args.first();
    const QString& text = args.at(1);
    QTextCodec* const codec = args.count() == 2 ? 0 : QTextCodec::codecForName( args.at(2).toLatin1() );

    if( codec == 0 && args.count() == 3 )
    {
        setError( UserDefinedError, tr( "Unknown text codec:").arg( args.at(2) ) );
        return false;
    }

    QFile file( fName );
    if( !file.open( QFile::Append | QFile::Text ) )
        {
        setError( UserDefinedError, tr("Cannot open %1 in Append mode: %2").arg(fName, file.errorString()) );
            return false;
        }
    QTextStream stream( &file );
    if( codec )
        stream.setCodec( codec );
    stream.setAutoDetectUnicode( true );
    stream << text;
    stream.flush();
    if( stream.status() != QTextStream::Ok )
    {
        setError( UserDefinedError, tr( "Error while writing data to %1" ).arg( fName ) );
        return false;
    }

    return true;
}

bool AppendFileOperation::undoOperation()
{
   // backupOfFile being empty -> file didn't exist before -> no error
    const QString filename = arguments().first();
    const QString backupOfFile = value( QLatin1String( "backupOfFile" ) ).toString();
    if( !backupOfFile.isEmpty() && !QFile::exists( backupOfFile ) )
    {
        setError( UserDefinedError, tr("Cannot find backup file for %1").arg(filename) );
        return false;
    }

    QFile file( filename );
    const bool removed = file.remove();
    if ( !removed ) {
        setError( UserDefinedError, tr("Could not restore backup file for %1: %2").arg(filename, file.errorString()) );
        return false;
    }

    // got deleted? We might be done, if it didn't exist before
    if( backupOfFile.isEmpty() )
        return true;

    QFile backupFile( backupOfFile );
    const bool success = backupFile.rename( filename );
    if ( !success )
        setError( UserDefinedError, tr("Could not restore backup file for %1: %2").arg(filename, backupFile.errorString()) );
    return success;
}

bool AppendFileOperation::testOperation()
{
    // TODO
    return true;
}

AppendFileOperation* AppendFileOperation::clone() const
{
    return UpdateOperation::clone< AppendFileOperation >();
}


////////////////////////////////////////////////////////////////////////////
// KDUpdater::PrependFileOperation
////////////////////////////////////////////////////////////////////////////

PrependFileOperation::PrependFileOperation()
{
    setName(QLatin1String( "PrependFile" ));
}

PrependFileOperation::~PrependFileOperation()
{

}

void PrependFileOperation::backup()
{
    const QString filename = arguments().first();

    QFile file( filename );
    if( !file.exists() )
        return; // nothing to backup

    setValue( QLatin1String( "backupOfFile" ), backupFileName( filename ) );
    if( !file.copy( value( QLatin1String( "backupOfFile" ) ).toString() ) )
    {
        setError( UserDefinedError, tr("Cannot backup file %1: %2").arg(filename, file.errorString()) );
        clearValue( QLatin1String( "backupOfFile" ) );
    }
}

bool PrependFileOperation::performOperation()
{
    // This operation takes two arguments. First argument is the name
    // of the file into which a text has to be appended. Second argument
    // is the text to preend.
    // Optionally, a third parameter defines the text codec to be used
    const QStringList args = this->arguments();
    if( args.count() != 2 && args.count() != 3 )
    {
        setError( InvalidArguments, tr("Invalid arguments: %1 arguments given, 2 or 3 expected.").arg( args.count() ) );
        return false;
    }

    const QString& fName = args.first();
    const QString& text = args.at(1);
    QTextCodec* const codec = args.count() == 2 ? 0 : QTextCodec::codecForName( args.at(2).toLatin1() );

    if( codec == 0 && args.count() == 3 )
    {
        setError( UserDefinedError, tr( "Unknown text codec:").arg( args.at(2) ) );
        return false;
    }

    /*
    If Append can append text into an empty file, Prepend should be
    able to prepend text into an empty file as well. 
    */

    // Load the file first.
    QString fContents;

    QFile file(fName);
    if( file.exists() ) // the file not existing is no problem
    {
        if( !file.open( QFile::ReadOnly | QFile::Text ) )
        {
            setError( UserDefinedError, tr( "Cannot open file %1 for reading: %2" ).arg( fName, file.errorString() ) );
        return false;
    }
        QTextStream stream( &file );
        if( codec )
            stream.setCodec( codec );
        stream.setAutoDetectUnicode( true );
        fContents = stream.readAll();
    file.close();
    } 

    // Prepend text to the file text
    fContents = text + fContents;

    // Now re-open the file in write only mode.
    if( !file.open( QFile::WriteOnly | QFile::Text ) )
        {
        setError( UserDefinedError, tr("Cannot open file %1 for writing: %2").arg(fName, file.errorString() ) );
            return false;
        }

    QTextStream stream( &file );
    if( codec )
        stream.setCodec( codec );
    stream.setAutoDetectUnicode( true );
    stream << fContents;
    stream.flush();
    if( stream.status() != QTextStream::Ok )
    {
        setError( UserDefinedError, tr( "Error while writing data to %1" ).arg( fName ) );
        return false;
    }

    return true;
}

bool PrependFileOperation::undoOperation()
{
    // bockupOfFile being empty -> file didn't exist before -> no error
    const QString filename = arguments().first();
    const QString backupOfFile = value( QLatin1String( "backupOfFile" ) ).toString();
    if( !backupOfFile.isEmpty() && !QFile::exists( backupOfFile ) )
    {
        setError( UserDefinedError, tr("Cannot find backup file for %1").arg(filename) );
        return false;
    }

    QFile file( filename );
    if ( !file.remove() ) {
        setError( UserDefinedError, tr("Cannot restore backup file for %1: %2").arg(filename, file.errorString()) );
        return false;
    }

    // got deleted? We might be done, if it didn't exist before
    if( backupOfFile.isEmpty() )
        return true;

    QFile backupF( backupOfFile );
    const bool success = backupF.rename( filename );
    if(!success)
        setError( UserDefinedError, tr("Cannot restore backup file for %1: %2").arg(filename, backupF.errorString()) );

    return success;
}

bool PrependFileOperation::testOperation()
{
    // TODO
    return true;
}

PrependFileOperation* PrependFileOperation::clone() const
{
    return UpdateOperation::clone< PrependFileOperation >();
}


////////////////////////////////////////////////////////////////////////////
// KDUpdater::ExecuteOperation
////////////////////////////////////////////////////////////////////////////

ExecuteOperation::ExecuteOperation()
{
    setName(QLatin1String( "Execute" ));
}

ExecuteOperation::~ExecuteOperation()
{

}

void ExecuteOperation::backup()
{
    // this is not possible, since the process can do whatever...
}

#if defined( SUPPORT_DETACHED_PROCESS_EXECUTION ) && defined( Q_WS_WIN )
// stolen from qprocess_win.cpp
static QString qt_create_commandline(const QString &program, const QStringList &arguments)
{
    QString args;
    if (!program.isEmpty()) {
        QString programName = program;
        if (!programName.startsWith(QLatin1Char('\"')) && !programName.endsWith(QLatin1Char('\"')) && programName.contains(QLatin1Char(' ')))
            programName = QLatin1Char('\"') + programName + QLatin1Char('\"');
        programName.replace(QLatin1Char('/'), QLatin1Char('\\'));

        // add the prgram as the first arg ... it works better
        args = programName + QLatin1Char(' ');
    }

    for (int i=0; i<arguments.size(); ++i) {
        QString tmp = arguments.at(i);
        // in the case of \" already being in the string the \ must also be escaped
        tmp.replace( QLatin1String("\\\""), QLatin1String("\\\\\"") );
        // escape a single " because the arguments will be parsed
        tmp.replace( QLatin1Char('\"'), QLatin1String("\\\"") );
        if (tmp.isEmpty() || tmp.contains(QLatin1Char(' ')) || tmp.contains(QLatin1Char('\t'))) {
            // The argument must not end with a \ since this would be interpreted
            // as escaping the quote -- rather put the \ behind the quote: e.g.
            // rather use "foo"\ than "foo\"
            QString endQuote(QLatin1Char('\"'));
            int i = tmp.length();
            while (i>0 && tmp.at(i-1) == QLatin1Char('\\')) {
                --i;
                endQuote += QLatin1Char('\\');
            }
            args += QLatin1String(" \"") + tmp.left(i) + endQuote;
        } else {
            args += QLatin1Char(' ') + tmp;
        }
    }
    return args;
}
#endif

bool ExecuteOperation::performOperation()
{
    // This operation receives only one argument. It is the complete
    // command line of the external program to execute.
    const QStringList args = this->arguments();
    if( args.isEmpty() )
    {
        setError( InvalidArguments, tr("Invalid arguments: no arguments given, at least one expected.") );
        return false;
    }

    bool success = false;
#ifdef SUPPORT_DETACHED_PROCESS_EXECUTION
    // unix style: when there's an ampersand after the command, it's started detached
    if( args.count() >= 2 && args.last() == QLatin1String( "&" ) )
    {
        args.pop_back();
#ifdef Q_WS_WIN
        QString arguments = qt_create_commandline( args.front(), args.mid( 1 ) );
        
        PROCESS_INFORMATION pinfo;

        STARTUPINFOW startupInfo = { sizeof( STARTUPINFO ), 0, 0, 0,
                                     static_cast< ulong >( CW_USEDEFAULT ), static_cast< ulong >( CW_USEDEFAULT ),
                                     static_cast< ulong >( CW_USEDEFAULT ), static_cast< ulong >( CW_USEDEFAULT ),
                                     0, 0, 0, STARTF_USESHOWWINDOW, SW_HIDE, 0, 0, 0, 0, 0
                                   };
        success = CreateProcess( 0, const_cast< wchar_t* >( static_cast< const wchar_t* >( arguments.utf16() ) ),
                                 0, 0, FALSE, CREATE_UNICODE_ENVIRONMENT | CREATE_NEW_CONSOLE, 0,
                                 0,
                                 &startupInfo, &pinfo );

#else
        success = QProcess::startDetached( args.front(), args.mid( 1 ) );
#endif
    }
    else
#endif
    {
        QProcess process;
        process.setReadChannelMode( QProcess::ForwardedChannels );
    	process.start( args.front(), args.mid( 1 ) );
    	success = process.waitForFinished( -1 );
    }
    if(!success)
        setError( UserDefinedError, tr("Cannot execute \"%1\"").arg( args.front() ) );

    return success;
}

/*!
 Cancels the ExecuteOperation. This methods tries to terminate the process
 gracefully by calling QProcess::terminate. After 10 seconds, the process gets killed.
 */
void ExecuteOperation::cancelOperation()
{
    
    if( process.state() == QProcess::Running )
        process.terminate();
    if( !process.waitForFinished( 10000 ) )
        process.kill();
}

void ExecuteOperation::readProcessOutput()
{
    QByteArray output = process.readAll();
    if (!output.isEmpty())
        emit outputChanged( QString::fromLocal8Bit( output.constData() ) );
}

bool ExecuteOperation::undoOperation()
{
    // this is not possible, since the process can do whatever...
    return false;
}

bool ExecuteOperation::testOperation()
{
    // TODO
    return true;
}

ExecuteOperation* ExecuteOperation::clone() const
{
    return UpdateOperation::clone< ExecuteOperation >();
}


////////////////////////////////////////////////////////////////////////////
// KDUpdater::UpdatePackageOperation
////////////////////////////////////////////////////////////////////////////

UpdatePackageOperation::UpdatePackageOperation()
{
    setName(QLatin1String( "UpdatePackage" ));
}

UpdatePackageOperation::~UpdatePackageOperation()
{

}

void UpdatePackageOperation::backup()
{
    const PackageInfo info = target()->packagesInfo()->packageInfo( target()->packagesInfo()->findPackageInfo( arguments().first() ) );
    setValue( QLatin1String( "oldVersion" ), info.version );
    setValue( QLatin1String( "oldDate" ), info.lastUpdateDate );
}

bool UpdatePackageOperation::performOperation()
{
    // This operation receives three arguments : the name of the package
    // the new version and the release date
    const QStringList args = this->arguments();
    if( args.count() != 3 )
    {
        setError( InvalidArguments, tr("Invalid arguments: %1 arguments given, 3 expected.").arg( args.count() ) );
        return false;
    }

    const QString& packageName = args.at( 0 );
    const QString& version = args.at( 1 );
    const QDate date = QDate::fromString( args.at( 2 ) );
    const bool success = target()->packagesInfo()->updatePackage( packageName, version, date );
    if(!success)
        setError( UserDefinedError, tr("Cannot update %1-%2").arg( packageName, version ) );

    return success;
}

bool UpdatePackageOperation::undoOperation()
{
    const QString packageName = arguments().first();
    const QString version = arguments().at( 1 );
    const QString oldVersion = value( QLatin1String( "oldVersion" ) ).toString();
    const QDate oldDate = value( QLatin1String( "oldDate" ) ).toDate();
    const bool success = target()->packagesInfo()->updatePackage( packageName, oldVersion, oldDate );
    if(!success)
        setError( UserDefinedError, tr("Cannot restore %1-%2").arg( packageName, version ) );

    return success;
}

bool UpdatePackageOperation::testOperation()
{
    // TODO
    return true;
}

UpdatePackageOperation* UpdatePackageOperation::clone() const
{
    return UpdateOperation::clone< UpdatePackageOperation >();
}


////////////////////////////////////////////////////////////////////////////
// KDUpdater::UpdateCompatOperation
////////////////////////////////////////////////////////////////////////////

UpdateCompatOperation::UpdateCompatOperation()
{
    setName(QLatin1String( "UpdateCompatLevel" ));
}

UpdateCompatOperation::~UpdateCompatOperation()
{

}

void UpdateCompatOperation::backup()
{
    setValue( QLatin1String( "oldCompatLevel" ), target()->packagesInfo()->compatLevel() );
}

bool UpdateCompatOperation::performOperation()
{
    // This operation receives one argument : the new compat level
    const QStringList args = this->arguments();
    if( args.count() != 1 )
    {
        setError( InvalidArguments, tr("Invalid arguments: %1 arguments given, 1 expected.").arg( args.count() ) );
        return false;
    }

    const int level = args.first().toInt();
    target()->packagesInfo()->setCompatLevel( level );
    return true;
}

bool UpdateCompatOperation::undoOperation()
{
    if( !hasValue( QLatin1String( "oldCompatLevel" ) ) )
    {
        setError( UserDefinedError, tr("Cannot restore previous compat-level") );
        return false;
    }

    target()->packagesInfo()->setCompatLevel( value( QLatin1String( "oldCompatLevel" ) ).toInt() );
    return true;
}

bool UpdateCompatOperation::testOperation()
{
    // TODO
    return true;
}

UpdateCompatOperation* UpdateCompatOperation::clone() const
{
    return UpdateOperation::clone< UpdateCompatOperation >();
}
