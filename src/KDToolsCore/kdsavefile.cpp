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

#include "kdsavefile.h"

#include <QtCore/QDebug>
#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QPointer>
#include <QtCore/QTemporaryFile>

#ifdef Q_OS_WIN
#include <io.h>
#include <windows.h>
#endif
#include <memory>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include <KDToolsCore/kdmetamethoditerator.h>

#ifdef Q_OS_WIN
#define DEFAULTBACKUPEXTENSION ".bak"
#else
#define DEFAULTBACKUPEXTENSION "~"
#endif

/*!
 Helper class allowing us to open a QTemporaryFile in other mode than ReadWrite.
 /internal
 */
class KDTemporaryFile : public QTemporaryFile
{
public:
    KDTemporaryFile( const QString& templateName )
        : QTemporaryFile( templateName )
    {
    }

    bool open( OpenMode flags )
    {
        return QTemporaryFile::open( flags );
    }
};

#ifdef Q_OS_WIN
void makeFileHidden( const QString& path, bool hidden )
{
    const QString absolute = QDir::toNativeSeparators( QFileInfo( path ).absoluteFilePath() );
    DWORD attributes = ::GetFileAttributes( (wchar_t*)absolute.utf16() );
    if( hidden )
        attributes |= FILE_ATTRIBUTE_HIDDEN;
    else
        attributes &= ~FILE_ATTRIBUTE_HIDDEN;
    ::SetFileAttributes( (wchar_t*)path.utf16(), attributes );
}
#endif

/*!
 Makes \a path absolute. If \a path is not yet absolute, it will be seen as relative to the
 current directory.
 \internal
 */
static QString makeAbsolute( const QString& path )
{
    if ( QDir::isAbsolutePath( path ) )
        return path;
    return QDir::current().absoluteFilePath( path );
}

/*!
 Creates and returns a temporary file, opened in mode \a m with permissions \a p.
 \internal
 */
static QFile* createFile( const QString& path, QIODevice::OpenMode m, QFile::Permissions p, bool & openOk, QFile::FileError & error )
{
    std::auto_ptr< KDTemporaryFile > file( new KDTemporaryFile( path ) );
    openOk = file->open( m );
    error = file->error();
    if ( !openOk ) // will be deleted (if already existing) automatically
        return 0;
    file->setAutoRemove( false );
    file->setPermissions( p );
    return file.release();
}

/*!
 Generates a temporary file name template based on target name \a path
 \internal
 */
static QString generateTempFileName( const QString& path, bool tmpPath )
{
    static const QString characters = QLatin1String( "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890" );
    const QFileInfo fi( path );
    const QString dir = tmpPath ? QDir::tempPath() : fi.absolutePath();
    const QString file = fi.fileName();

#ifndef Q_OS_WIN
    const QString tmp = QLatin1String( "%1/.%2.tmp.XXXXXX" );
#else
    const QString tmp = QLatin1String( "%1/%2.tmp.XXXXXX" );
#endif
    return tmp.arg( dir, file );
}

/*!
  \class KDSaveFile KDSaveFile
  \ingroup core
  \brief Secure and robust writing to a file

  KDSaveFile is used to write new files in a secure and robust way. It makes sure
  the file is really created at its target location and not on a different place by "overwriting"
  a softlink. Additionally, it is exception-safe and robust, i.e. KDSaveFile makes sure that all your
  data is written to the target or nothing at all. A backup of an existing file can be kept.

  \section general-use General Use

  The general use case is when a program writes data to a file it wants to create. After creating
  the KDSaveFile by passing the target file name, it can be used like a "normal" QFile instance. As
  soon as it has been opened in read/write and write-only mode, data can be written to it. The only
  difference is that the file's contents have to be committed by calling commit on the instance after
  all data have been written to it.

  See this simple code snippet as example:

  \code
  KDSaveFile file( QLatin1String( "myfile.dat" ) );
  file.open( QIODevice::WriteOnly );
  QDataStream stream( &file );
  stream << ...; // write whatever...
  file.commit();
  \endcode

  \section advantages Advantages

  \subsection exception-safety Exception Safety

  KDSaveFile is exception safe. If there's an exception thrown during writing into KDSaveFile, i.e. between
  open() and commit(), all changes made to the file are silently ignored and the file is not left in a
  broken state.

  \subsection softlinks Know where contents gets written

  KDSaveFile makes sure that the data are really written to a new file. By just opening a file in write
  mode, it might be possible that, in case of a softlink, the data gets written somewhere else, even into
  a device node on Unix. To avoid that, KDSaveFile writes all content into a temporary file first and
  renames this file to the actual file name when committing. If the file was already existing, it gets
  backuped, if requested.
*/

/*!
 \enum KDSaveFile::CommitMode
 This enum is used with commit() to describe what KDSaveFile should do with an existing file with the
 same name.
 */

/*!
 \var KDSaveFile::BackupExistingFile
 Backups the existing file appending the extension as returned by backupExtension().
 */

/*!
 \var KDSaveFile::OverwriteExistingFile
 Overwrites the existing file.
 */

/*!
 \internal
 */
class KDSaveFile::Private
{
public:
    explicit Private( const QString& fname, KDSaveFile* qq )
        : q( qq ),
          backupExtension( QLatin1String( DEFAULTBACKUPEXTENSION ) ),
          permissions( QFile::ReadUser | QFile::WriteUser ),
          filename( fname ),
          error( QFile::NoError )
    {
        //TODO respect umask instead of hardcoded default permissions
    }

    ~Private()
    {
        deleteTempFile();
    }

private:
    KDSaveFile* const q;

public:
    /*!
     Deletes the temporary file.
     \internal
     */
    bool deleteTempFile()
    {
        if( !tmpFile )
            return true;
        const QString name = tmpFile->fileName();
        delete tmpFile;
        //force a real close by deleting the object, before deleting the actual file. Needed on Windows
        return QFile::remove( name );
    }

    /*!
     (Re)-creates the temporary file and opens it in \a mode.
     \internal
     */
    bool recreateTemporaryFile( QIODevice::OpenMode mode )
    {
        deleteTempFile();
        bool ok = false;
        // first try, create it in the target directory
        tmpFile = createFile( generateTempFileName( filename, false ), mode, permissions, ok, error );

        // second try, create it in the temp directory
        if( tmpFile == 0 )
            tmpFile = createFile( generateTempFileName( filename, true ), mode, permissions, ok, error );

        if( tmpFile != 0 )
        {
            KDMetaMethodIterator it( QIODevice::staticMetaObject, KDMetaMethodIterator::Signal, KDMetaMethodIterator::IgnoreQObjectMethods );
            while( it.hasNext() )
            {
                it.next();
                connect( tmpFile, it.connectableSignature(), q, it.connectableSignature() );
            }
#ifdef Q_OS_WIN
            makeFileHidden( tmpFile->fileName(), true );
#endif
        }
        return tmpFile != 0 && ok;
    }

    /*!
     Generates a backup file name based on the file name this instance
     is working with.
     \internal
     */
    QString generateBackupName() const
    {
        const QString bf = filename + backupExtension;
        if ( !QFile::exists( bf ) )
            return bf;
        int count = 1;
        while( QFile::exists(  bf + QString::number( count ) ) )
            ++count;
        return bf + QString::number( count );
    }

    /*!
     Propagates the error string form the internal temporary file to this instance, if any.
     \internal
     */
    void propagateErrors()
    {
        if ( !tmpFile )
            return;
        q->setErrorString( tmpFile->errorString() );
    }

    QString backupExtension;
    QFile::Permissions permissions;
    QString filename;
    QPointer<QFile> tmpFile;
    QFile::FileError error;
};

/*!
 Creates a new KDSaveFile instance with \a parent.
 */
KDSaveFile::KDSaveFile( QObject* parent )
    : QIODevice( parent ),
      d( new Private( QString(), this ) )
{
}

/*!
 Creates a new KDSaveFile instance working on \a filename with \a parent.
 */
KDSaveFile::KDSaveFile( const QString& filename, QObject* parent )
    : QIODevice( parent ),
      d( new Private( makeAbsolute( filename ), this ) )
{
}

/*!
 Destroys this KDSaveFile. If commit was not called yet, all changes to the file are lost.
 */
KDSaveFile::~KDSaveFile()
{
}

/*!
 Returns the file error status.
*/
QFile::FileError KDSaveFile::error() const
{
    if( d->tmpFile != 0 )
        return d->tmpFile->error();
    else
        return d->error;
}

/*!
 Sets the file's error to QFile::NoError.
 */
void KDSaveFile::unsetError()
{
    if( d->tmpFile != 0 )
        d->tmpFile->unsetError();
    d->error = QFile::NoError;
}

/*!
 Closes the file without commit any changes. All changes to the file are lost.
 */
void KDSaveFile::close()
{
    d->deleteTempFile();
    QIODevice::close();
}

/*!
 Opens this file in \a mode.
 Note that only QIODevice::ReadWrite and QIODevice::WriteOnly are supported.
 */
bool KDSaveFile::open( OpenMode mode )
{
    setOpenMode( QIODevice::NotOpen );
    if ( mode & QIODevice::Append )
    {
        setErrorString( tr("Append mode not supported.") );
        return false;
    }

    if ( ( mode & QIODevice::WriteOnly ) == 0 ) {
        setErrorString( tr("Read-only access not supported.") );
        return false;
    }

    const QFile f( d->filename );

    if ( f.exists() && !QFileInfo( d->filename ).isWritable() ) {
        setErrorString( tr("The target file %1 exists and is not writable").arg( d->filename ) );
        return false;
    }
    const bool opened = d->recreateTemporaryFile( mode );
    if( opened )
        setOpenMode( mode );

    if( f.exists() )
        d->tmpFile->setPermissions( f.permissions() );

    //if target file already exists, apply permissions of existing file to temp file
    return opened;
}

/*!
 \reimp
 */
qint64 KDSaveFile::bytesAvailable() const
{
    if( !isSequential() )
        return QIODevice::bytesAvailable();
    else
        return d->tmpFile ? d->tmpFile->bytesAvailable() : QIODevice::bytesAvailable();
}

/*!
 \reimp
 */
qint64 KDSaveFile::bytesToWrite() const
{
    return ( d->tmpFile ? d->tmpFile->bytesToWrite() : 0 ) + QIODevice::bytesToWrite();
}

/*!
 \reimp
 */
bool KDSaveFile::canReadLine() const
{
    bool can = QIODevice::canReadLine();
    if( !can && d->tmpFile )
    {
        const qint64 p = d->tmpFile->pos();
        char c;
        while( !can && d->tmpFile->getChar( &c ) )
            can = c == '\n';
        d->tmpFile->seek( p );
    }
    return can;
}

/*!
 \reimp
 */
bool KDSaveFile::isSequential() const
{
    return d->tmpFile ? d->tmpFile->isSequential() : QIODevice::isSequential();
}

/*!
 \reimp
 */
bool KDSaveFile::seek( qint64 pos )
{
    const bool ret = d->tmpFile ? d->tmpFile->seek( pos ) : QIODevice::seek( pos );
    if ( !ret )
        d->propagateErrors();
    else
        QIODevice::seek( pos );
    return ret;
}

/*!
 \reimp
 */
qint64 KDSaveFile::size() const
{
    return d->tmpFile ? d->tmpFile->size() : QIODevice::size();
}

/*!
 \reimp
 */
bool KDSaveFile::waitForBytesWritten( int msecs )
{
    return d->tmpFile ? d->tmpFile->waitForBytesWritten( msecs ) : QIODevice::waitForBytesWritten( msecs );
}

/*!
 \reimp
 */
bool KDSaveFile::waitForReadyRead( int msecs )
{
    return d->tmpFile ? d->tmpFile->waitForReadyRead( msecs ) : QIODevice::waitForReadyRead( msecs );
}

/*!
 Commits the contents written into this KDSaveFile to the target using \a mode.
 Returns true on success, otherwise false.
 The file will be closed during commit.
 \sa CommitMode
 */
bool KDSaveFile::commit( KDSaveFile::CommitMode mode )
{
    if( !d->tmpFile )
        return false;

    const QString tmpfname = d->tmpFile->fileName();
    flush();
    delete d->tmpFile;

    // first step: backup the existing file (if any)
    QFile orig( d->filename );
    QString backup;
    if( orig.exists() )
    {
        backup = d->generateBackupName();
        if( !orig.rename( backup ) )
        {
            setErrorString( tr("Could not backup existing file %1: %2").arg( d->filename, orig.errorString() ) );
            if ( mode != OverwriteExistingFile )
                return false;
        }
        orig.setFileName( d->filename );
        if( orig.exists() && !orig.remove() )
        {
            setErrorString( tr( "Could not remove existing file %1: %2" ).arg( d->filename, orig.errorString() ) );
            return false;
        }
    }

    // second step: rename the temp file to the target file name
    QFile target( tmpfname );
    if( !target.rename( d->filename ) )
    {
        setErrorString( target.errorString() );
        return false;
    }

#ifdef Q_OS_WIN
    makeFileHidden( d->filename, false );
#endif

    // third step, if the existing file is to be overwritten: remove the backup we created in first step
    if( mode == OverwriteExistingFile )
    {
        QFile tmp( backup );
        const bool removed = !tmp.exists() || tmp.remove( backup );
        if ( !removed )
            qWarning() << "Could not remove the backup: " << tmp.errorString();
    }

    QIODevice::close();

    return true;
}

/*!
 Returns the name set by setFileName() or passed to the KDSaveFile constructor.
 */
QString KDSaveFile::fileName() const
{
    return d->filename;
}

/*!
 Sets the \a filename of the file. The name can have no path, a relative path, or an absolute path.
 If the file was opened before, it will be closed. All changes made to the file will be lost, then.
 */
void KDSaveFile::setFileName( const QString& filename )
{
    const QString fn = makeAbsolute( filename );
    if( fn == d->filename )
        return;
    close();
    delete d->tmpFile;
    d->filename = fn;
}

/*!
 \reimp
 */
qint64 KDSaveFile::readData( char* data, qint64 maxSize )
{
    if( !d->tmpFile )
    {
        setErrorString( tr( "Could not read from file: Temporary file does not exist." ) );
        return -1;
    }
    const qint64 ret = d->tmpFile->read( data, maxSize );
    d->propagateErrors();
    return ret;
}

/*!
 \reimp
 */
qint64 KDSaveFile::readLineData( char* data, qint64 maxSize )
{
    if( !d->tmpFile )
    {
        setErrorString( tr( "Could not read from file: Temporary file does not exist." ) );
        return -1;
    }
    const qint64 ret = d->tmpFile->readLine( data, maxSize );
    d->propagateErrors();
    return ret;
}

/*!
 \reimp
 */
qint64 KDSaveFile::writeData( const char* data, qint64 maxSize )
{
    if( !d->tmpFile )
    {
        setErrorString( tr( "Could not write to file: Temporary file does not exist." ) );
        return -1;
    }
    const qint64 ret = d->tmpFile->write( data, maxSize );
    d->propagateErrors();
    return ret;
}

/*!
 Flushes any buffered data to the file. Returns true if successful; otherwise returns false.
 */
bool KDSaveFile::flush()
{
    return d->tmpFile ? d->tmpFile->flush() : false;
}

/*!
 Sets the file size (in bytes) to \a sz. Returns true if the file could be resized; false otherwise.
 */
bool KDSaveFile::resize( qint64 sz )
{
    return d->tmpFile ? d->tmpFile->resize( sz ) : false;
}

/*!
 Returns the file handle of the file.
 */
int KDSaveFile::handle() const
{
    return d->tmpFile ? d->tmpFile->handle() : -1;
}

/*!
 Returns the complete access permissions of this file.
 \sa QFile::Permission
 */
QFile::Permissions KDSaveFile::permissions() const
{
    return d->tmpFile ? d->tmpFile->permissions() : d->permissions;
}

/*!
 Set this file's access permissions to \a p.
 */
bool KDSaveFile::setPermissions( QFile::Permissions p )
{
    d->permissions = p;
    if ( d->tmpFile )
        return d->tmpFile->setPermissions( p );
    return false;
}

/*!
 Sets the file extension used to create a backup file name.
 This defaults to ".bak" on Windows, "~" on any other system.
 */
void KDSaveFile::setBackupExtension( const QString& ext )
{
    d->backupExtension = ext;
}

/*!
 Returns he file extension used to create a backup file name.
 \sa setBackupExtension
 */
QString KDSaveFile::backupExtension() const
{
    return d->backupExtension;
}

#ifdef KDTOOLSCORE_UNITTESTS

#include <KDUnitTest/Test>

#include <QUuid>

static qint64 blockingWrite( QIODevice& dev, const QByteArray& data )
{
    qint64 written = 0;
    while( written < data.size() )
    {
        const qint64 n = dev.write( data.data() + written, data.size() - written );
        if( n < 0 )
            return -1;
        written += n;
    }
    return data.size();
}

static QByteArray blockingRead( QIODevice& dev, qint64 count )
{
    qint64 read = 0;
    QByteArray data;
    data.resize( count );
    while( read < count )
    {
        const qint64 n = dev.read( data.data(), count - read );
        if( n < 0 )
            return QByteArray();
        read += n;
    }
    return data;
}

static std::ostream& operator<<( std::ostream& stream, const QByteArray& ba )
{
    stream << "QByteArray( " << ba.data() << " )";
    return stream;
}


KDAB_UNITTEST_SIMPLE( KDSaveFile, "kdtools/core" ) {
    const QString filename = QString::fromLatin1( "kdsavefile-test%1" ).arg( QUuid::createUuid().toString() );

    {
        // first make sure that the file doesn't exist...
        assertTrue( !QFile::exists( filename ) || QFile::remove( filename ) );

        const QString testfile1 = filename;
        QByteArray testData("lalalala");
        KDSaveFile saveFile( testfile1 );
        assertTrue( saveFile.open( QIODevice::WriteOnly ) );
        assertEqual( blockingWrite( saveFile, testData ), testData.size() );
        assertTrue( saveFile.commit() );
        assertEqual( QFile( testfile1 ).size(), testData.count() );
        assertTrue( QFile::exists( testfile1 ) );
        QFile f( testfile1 );
        f.open( QIODevice::ReadOnly );
        assertEqual( blockingRead( f, testData.count() ), testData );
        f.close();

    }
    {
        const QString testfile1 = filename;
        QByteArray testData("lalalala");
        QByteArray newTestData("lalalalalalalalalala");
        KDSaveFile saveFile( testfile1 );
        assertTrue( saveFile.open( QIODevice::WriteOnly ) );
        saveFile.write( newTestData.constData(), newTestData.size() );
        saveFile.close();
        QFile f( testfile1 );
        f.open( QIODevice::ReadOnly );
        assertEqual( blockingRead( f, testData.count() ), testData );
        assertTrue( f.remove() );
    }
    {
        const QString testfile1 = filename;
        QByteArray testData("lalalala");
        KDSaveFile saveFile( testfile1 );
        assertTrue( saveFile.open( QIODevice::WriteOnly ) );
        saveFile.write( testData.constData(), testData.size() );
        saveFile.close();
        assertFalse( QFile::exists( testfile1 ) );
    }
    {
        const QString testfile1 = filename;
        QByteArray testData("lalalala");
        KDSaveFile saveFile( testfile1 );
        assertTrue( saveFile.open( QIODevice::WriteOnly ) );
        saveFile.write( testData.constData(), testData.size() );
        assertTrue( saveFile.commit() );
        assertTrue( QFile::exists( testfile1 ) );

        KDSaveFile sf2( testfile1 );
        sf2.setBackupExtension( QLatin1String(".bak") );
        assertTrue( sf2.open( QIODevice::WriteOnly ) );
        sf2.write( testData.constData(), testData.size() );
        sf2.commit(); //commit in backup mode (default)
        const QString backup = testfile1 + sf2.backupExtension();
        assertTrue( QFile::exists( backup ) );
        assertTrue( QFile::remove( backup ) );

        KDSaveFile sf3( testfile1 );
        sf3.setBackupExtension( QLatin1String(".bak") );
        assertTrue( sf3.open( QIODevice::WriteOnly ) );
        sf3.write( testData.constData(), testData.size() );
        sf3.commit( KDSaveFile::OverwriteExistingFile );
        const QString backup2 = testfile1 + sf3.backupExtension();
        assertFalse( QFile::exists( backup2 ) );

        assertTrue( QFile::remove( testfile1 ) );
    }
    {
        const QString testfile1 = filename;
        KDSaveFile sf( testfile1 );
        assertFalse( sf.open( QIODevice::ReadOnly ) );
        assertFalse( sf.open( QIODevice::WriteOnly|QIODevice::Append ) );
        assertTrue( sf.open( QIODevice::ReadWrite ) );

        const QByteArray testData("lalalala");
        sf.write( testData.constData(), testData.size() );
        assertEqual( sf.pos(), testData.size() );
        sf.seek( 0 );
        assertEqual( sf.pos(), 0 );
        assertEqual( sf.bytesAvailable(), testData.size() );
        char c = '\0';
        assertTrue( sf.getChar( &c ) );
        assertEqual( sf.pos(), 1 );
        assertEqual( c, 'l' );
        assertEqual( sf.bytesAvailable(), testData.size() - 1 );
        sf.ungetChar( c );
        assertEqual( sf.pos(), 0 );
        assertEqual( sf.bytesAvailable(), testData.size() );
        assertFalse( sf.canReadLine() );

        const QByteArray testLine( "I'm a nice line\r\nI'm another one.\r\n" );
        assertTrue( sf.seek( 0 ) );
        assertEqual( sf.pos(), 0 );
        assertEqual( blockingWrite( sf, testLine ), testLine.size() );
        sf.seek( 0 );
        assertEqual( blockingRead( sf, testLine.count() ), testLine );
        sf.seek( 0 );
        assertEqual( sf.bytesAvailable(), testLine.size() );
        assertEqual( sf.size(), testLine.size() );
        assertTrue( sf.canReadLine() );
        assertEqual( sf.pos(), 0 );
    }
}

#endif // KDTOOLSCORE_UNITTESTS
