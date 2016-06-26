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

#include <QDebug>
#include <QApplication>
#include <QEventLoop>
#include <QSignalSpy>
#include <QTest>
#include <QTimer>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QDirIterator>
#include <QProcess>
#include <QDomDocument>
#include <QTextStream>

/*
Right now we are only testing the performOperation()
implementation in UpdateOperations. backup and undo
will be tested in a future revision of this test program
*/

#include <KDUpdater/UpdateOperation>
#include <KDUpdater/UpdateOperationFactory>

class UpdateOperationWrapper
{
public:
    UpdateOperationWrapper(KDUpdater::UpdateOperation** oper)
        : m_operation(oper) { }
    ~UpdateOperationWrapper() {
        if(m_operation && *m_operation)
        {
            delete *m_operation;
            *m_operation = 0;
        }
    }

private:
    KDUpdater::UpdateOperation** m_operation;
};

// just make stuff public for us to test
class PseudoUpdateOperation : public KDUpdater::UpdateOperation
{
public:
    void setArguments( const QStringList& arguments ) { KDUpdater::UpdateOperation::setArguments( arguments ); }

    void setValue( const QString& name, const QVariant& value ) { KDUpdater::UpdateOperation::setValue( name, value ); }
    QVariant value( const QString& name ) const { return KDUpdater::UpdateOperation::value( name ); }
    bool hasValue( const QString& name ) const { return KDUpdater::UpdateOperation::hasValue( name ); }
    void clearValue( const QString& name ) { KDUpdater::UpdateOperation::clearValue( name ); }

    void backup() {}
    bool performOperation() { return false; }
    bool undoOperation() { return false; }
    bool testOperation() { return false; }
    PseudoUpdateOperation* clone() const { return KDUpdater::UpdateOperation::clone< PseudoUpdateOperation >(); }
};


#define FETCH_UPDATE_OPERATION(OperationName, Arguments) \
KDUpdater::UpdateOperation* oper = \
    KDUpdater::UpdateOperationFactory::instance().create( QLatin1String( #OperationName ), (Arguments), 0); \
UpdateOperationWrapper operWrapper(&oper); \
QVERIFY2(oper!=0, #OperationName " operation was not found");


class UpdateOperationsTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testCopy();
    void testMove();
    void testDelete();
    void testMkdir();
    void testRmDir();
    void testAppendFile();
    void testPrependFile();
    void testExecute();
    void testXml();

private:
    QDir testDirectory(const QString& operType);
    bool createFile(const QString& fileName, const QString& contents);
    bool readFile(const QString& fileName, QString& contents);
    void printError(KDUpdater::UpdateOperation* oper);
    bool executeProgram(const QString& program, QString& output);

private:
    QString m_tempDirectory;
};

/**
 * Recursively removes the directory at \a path.
 * This function was copied form kdupdaterupdateoperations.cpp
 */
static bool removeDirectory( const QString& path )
{
    static const QString dot = QString::fromLatin1( "." );
    static const QString dotdot = QString::fromLatin1( ".." );

    QDirIterator it( path );
    while( it.hasNext() )
    {
        const QString filePath = it.next();
        const QString fileName = it.fileName();
        if( fileName == dot || fileName == dotdot || filePath.isEmpty() )
            continue;

        const QFileInfo fi( filePath );
        if( fi.isFile() )
        {
            if( !QFile::remove( filePath ) )
                return false;
        }
        else if( fi.isDir() )
        {
            if( !removeDirectory( filePath ) )
                return false;
        }
    }
    const QFileInfo fi( path );
    if( !fi.dir().rmdir( fi.fileName() ) )
        return false;

    return true;
}

void UpdateOperationsTest::initTestCase()
{
    // Create a temporary directory within which
    // all the test case sample files will be created
    m_tempDirectory = QString::fromLatin1("%1/UpdateOperationsTest").arg(QDir::tempPath());

    QDir temp = QDir::temp();
    temp.mkdir( QLatin1String( "UpdateOperationsTest" ) );

    qDebug("Using test directory as %s", qPrintable(m_tempDirectory));
}

void UpdateOperationsTest::cleanupTestCase()
{
    // Recursively delete the temporary directory
    removeDirectory( m_tempDirectory );
}

void UpdateOperationsTest::testCopy()
{
    const QDir testDir = testDirectory( QLatin1String( "Copy" ) );

    // Case #1: Lets try with insufficient number of arguments
    {
        FETCH_UPDATE_OPERATION(Copy, QStringList() );
        bool success = oper->performOperation();
        printError(oper);
        QVERIFY2(!success, "Insufficient arguments given, but copy succeeded");
    }

    // Case #2: Lets try with insufficient number of arguments
    {
        QStringList args;
        args << testDir.absoluteFilePath( QLatin1String( "source.txt" ) );
        FETCH_UPDATE_OPERATION(Copy, args);
        bool success = oper->performOperation();
        printError(oper);
        QVERIFY2(!success, "Insufficient arguments given, but copy succeeded");
    }


    // Case #3: Lets try to copy source to destination,
    // where the source does not exist
    {
        QStringList args;
        args << testDir.absoluteFilePath( QLatin1String( "source.txt" ) );
        args << testDir.absoluteFilePath( QLatin1String( "destination.txt" ) );
        FETCH_UPDATE_OPERATION(Copy, args);
        bool success = oper->performOperation();
        printError(oper);
        QVERIFY2(!success, "Copy succeeded even when source did not exist");
    }

    // Case #4: Lets try to copy source to destination
    {
        // Create a simple source file
        const QString sourceFile = testDir.absoluteFilePath( QLatin1String( "source.txt" ) );
        const QString sourceFileContents( QLatin1String( "Some simple content" ) );
        createFile(sourceFile, sourceFileContents);

        const QString destFile = testDir.absoluteFilePath( QLatin1String( "destination.txt" ) );
        QString destFileContents;

        const QStringList args = QStringList() << sourceFile << destFile;
        FETCH_UPDATE_OPERATION(Copy, args);
        bool success = oper->performOperation();
        printError(oper);
        QVERIFY2(success, "Copy was not successful even when source existed "
                          "and destination did not");

        readFile(destFile, destFileContents);
        QVERIFY2(sourceFileContents == destFileContents,
                 "Copy was not accurate. Destination contains differnt content");
    }

    // Case #5: Lets try to copy source to an existing destination
    {
        // We know at this point that both source.txt and destination.txt
        // exist. So, lets just run the copy operation again. It should
        // succeed, because the copy operation should delete existing file
        // and create a new copy.
        // Create a simple source file
        const QString sourceFile = testDir.absoluteFilePath( QLatin1String( "source.txt" ) );
        const QString sourceFileContents = QLatin1String( "Changed content" ) ;
        createFile(sourceFile, sourceFileContents);

        const QString destFile = testDir.absoluteFilePath( QLatin1String( "destination.txt" ) );
        QString destFileContents;

        const QStringList args = QStringList() << sourceFile << destFile;
        FETCH_UPDATE_OPERATION(Copy, args);
        bool success = oper->performOperation();
        printError(oper);

        QVERIFY2(success, "Copy did not succeed while overwriting an existing file");

        readFile(destFile, destFileContents);
        QVERIFY2(sourceFileContents == destFileContents,
                 "Copy was not accurate. Destination contains differnt content");
    }
}

void UpdateOperationsTest::testMove()
{
    const QDir testDir = testDirectory( QLatin1String( "Move" ) );

    // Case #1: Test insufficient arguments
    {
        FETCH_UPDATE_OPERATION(Move, QStringList());
        bool success = oper->performOperation();
        printError(oper);
        QVERIFY2(!success, "Move succeeded even when no arguments were provided");
    }

    // Case #2: Test insufficient arguments
    {
        FETCH_UPDATE_OPERATION(Move, QStringList() << testDir.absoluteFilePath( QLatin1String( "Sample.txt" ) ) );
        bool success = oper->performOperation();
        printError(oper);
        QVERIFY2(!success, "Move succeeded even when only one argument was specified");
    }

    // Case #3: Test a valid source -> destination move
    {
        const QString sourceFile = testDir.absoluteFilePath( QLatin1String( "source.txt" ) );
        const QString sourceFileContents =  QLatin1String( "Sample text file" ) ;
        const QString destFile = testDir.absoluteFilePath( QLatin1String( "destination.txt" ) );
        QString destFileContents;
        createFile(sourceFile, sourceFileContents);

        FETCH_UPDATE_OPERATION(Move, QStringList() << sourceFile << destFile);
        bool success = oper->performOperation();
        printError(oper);
        QVERIFY2(success, "Move operation was not successful");
        QVERIFY2(QFile::exists(destFile), "Destination file not found");
        QVERIFY2(!QFile::exists(sourceFile), "Source file found even after moving");

        readFile(destFile, destFileContents);
        QVERIFY2(sourceFileContents == destFileContents,
                 "Contents of moved file different");
    }

    // Case #4: Move non-existing source to existing destination
    {
        const QString sourceFile = testDir.absoluteFilePath( QLatin1String( "source.txt" ) );
        const QString destFile = testDir.absoluteFilePath( QLatin1String( "destination.txt" ) );

        FETCH_UPDATE_OPERATION(Move, QStringList() << sourceFile << destFile );
        bool success = oper->performOperation();
        printError(oper);
        QVERIFY2(!success, "Move operation successful, when source did not exist");
    }

    // Case #5: Move a file over an existing file
    {
        const QString sourceFile = testDir.absoluteFilePath( QLatin1String( "source.txt" ) );
        const QString sourceFileContents =  QLatin1String( "Sample text file" ) ;
        const QString destFile = testDir.absoluteFilePath( QLatin1String( "destination.txt" ) );
        QString destFileContents;
        createFile(sourceFile, sourceFileContents);

        FETCH_UPDATE_OPERATION(Move, QStringList() << sourceFile << destFile);
        bool success = oper->performOperation();
        printError(oper);
        QVERIFY2(success, "Move operation was not successful");
        QVERIFY2(QFile::exists(destFile), "Destination file not found");
        QVERIFY2(!QFile::exists(sourceFile), "Source file found even after moving");

        readFile(destFile, destFileContents);
        QVERIFY2(sourceFileContents == destFileContents,
                 "Contents of moved file different");
    }
}

void UpdateOperationsTest::testDelete()
{
    QDir testDir = testDirectory( QLatin1String( "Delete" ) );

    // Case #1: Insufficient arguments
    {
        FETCH_UPDATE_OPERATION(Delete, QStringList());
        bool success = oper->performOperation();
        printError(oper);
        QVERIFY2(!success, "Delete succeeded even when no arguments were provided");
    }

    // Case #2: Delete a non-existing file
    {
        FETCH_UPDATE_OPERATION(Delete, QStringList() << testDir.absoluteFilePath( QLatin1String( "Sample.txt" ) ) );
        bool success = oper->performOperation();
        printError(oper);
        QVERIFY2(!success, "Delete successed in deleting a non-existing file");
    }

    // Case #3: Delete an existing file
    {
        const QString fileName = testDir.absoluteFilePath( QLatin1String( "Sample.txt" ) );
        createFile(fileName,  QLatin1String( "This is a simple file" ) );

        FETCH_UPDATE_OPERATION(Delete, QStringList() << fileName);
        bool success = oper->performOperation();
        printError(oper);
        QVERIFY2(success, "Delete could not delete an existing file");
        QVERIFY2(!QFile::exists(fileName), "Delete did not actually delete the file");
    }

    // Case #4: Delete a directory file
    {
        testDir.mkdir( QLatin1String( "NewDirectory" ) );
        const QString fileName = testDir.absoluteFilePath( QLatin1String( "NewDirectory" ) );

        FETCH_UPDATE_OPERATION(Delete, QStringList() << fileName );
        bool success = oper->performOperation();
        printError(oper);
        QVERIFY2(!success, "Delete successfully deleted a directory!");
    }
}

void UpdateOperationsTest::testMkdir()
{
    const QDir testDir = testDirectory( QLatin1String( "MkDir" ) );

    // Case #1: Insufficient arguments
    {
        FETCH_UPDATE_OPERATION(Mkdir, QStringList());
        bool success = oper->performOperation();
        printError(oper);
        QVERIFY2(!success, "Mkdir succeeded with insufficient arguments");
    }

    // Case #2: Create a proper directory
    {
        const QString dirName = testDir.absoluteFilePath( QLatin1String( "NewDirectory" ) );
        FETCH_UPDATE_OPERATION(Mkdir, QStringList() << dirName );
        bool success = oper->performOperation();
        QVERIFY2(success, "Mkdir did not create the directory");

        QFileInfo fi(dirName);
        QVERIFY2(fi.exists() && fi.isDir(), "Directory was not actually created");
    }

    // Case #3: Create an existing directory
    {
        const QString dirName = testDir.absoluteFilePath( QLatin1String( "NewDirectory" ) );
        FETCH_UPDATE_OPERATION(Mkdir, QStringList() << dirName );
        bool success = oper->performOperation();
        QVERIFY2(success, "Mkdir created a directory that already existed");
    }
}

void UpdateOperationsTest::testRmDir()
{
    QDir testDir = testDirectory( QLatin1String( "Rmdir" ) );

    // Case #1: Insufficient arguments
    {
        FETCH_UPDATE_OPERATION(Rmdir, QStringList());
        bool success = oper->performOperation();
        printError(oper);
        QVERIFY2(!success, "Rmdir succeeded with insufficient arguments");
    }

    // Case #2: Delete a non-existing directory
    {
        const QString dirName = testDir.absoluteFilePath( QLatin1String( "ExistingDir" ) );
        FETCH_UPDATE_OPERATION(Rmdir, QStringList() << dirName );
        bool success = oper->performOperation();
        printError(oper);
        QVERIFY2(!success, "Rmdir deleted a non-existing directory!");
    }

    // Case #3: Delete an existing directory
    {
        const QString dirName = testDir.absoluteFilePath( QLatin1String( "ExistingDir" ) );
        testDir.mkdir( QLatin1String( "ExistingDir" ) );
        FETCH_UPDATE_OPERATION(Rmdir, QStringList() << dirName );
        bool success = oper->performOperation();
        printError(oper);
        QVERIFY2(success, "Rmdir did not delete a directory");

        QFileInfo fi(dirName);
        QVERIFY2(!fi.exists(), "Rmdir did not actually delete a directory");
    }

    // Case #4: Delete a non-empty directory
    {

        const QString dirName = testDir.absoluteFilePath( QLatin1String( "ExistingDir" ) );
        testDir.mkdir( QLatin1String( "ExistingDir" ) );
        testDir.cd( QLatin1String( "ExistingDir" ) );
        createFile( testDir.absoluteFilePath( QLatin1String( "Sample.txt" ) ),
                    QLatin1String( "Some file contents" )  );

        FETCH_UPDATE_OPERATION(Rmdir, QStringList() << dirName );
        bool success = oper->performOperation();
        printError(oper);
        QVERIFY2(!success, "Rmdir deleted a non-empty directory");

        QFileInfo fi(dirName);
        QVERIFY2(fi.exists(), "Rmdir did delete a directory that it reported not to have deleted");
    }

    // Case #5: Use Rmdir on a file, not directory
    {
        const QString fileName = testDir.absoluteFilePath( QLatin1String( "ExistingDir/Sample.txt" ) );
        FETCH_UPDATE_OPERATION(Rmdir, QStringList() << fileName );
        bool success = oper->performOperation();
        printError(oper);
        QVERIFY2(!success, "Rmdir deleted a file!");
    }
}

void UpdateOperationsTest::testAppendFile()
{
    const QDir testDir = testDirectory( QLatin1String( "AppendFile" ) );

    // Case #1: Iinsufficient args
    {
        FETCH_UPDATE_OPERATION(AppendFile, QStringList());
        bool success = oper->performOperation();
        printError(oper);
        QVERIFY2(!success, "AppendFile succeeded with insufficient arguments");
    }

    // Case #2: Iinsufficient args
    {
        FETCH_UPDATE_OPERATION(AppendFile, QStringList() << testDir.absoluteFilePath( QLatin1String( "Sample.txt" ) ));
        bool success = oper->performOperation();
        printError(oper);
        QVERIFY2(!success, "AppendFile succeeded with insufficient arguments");
    }

    // Case #3: Valid append into a new file
    {
        const QString fileName = testDir.absoluteFilePath( QLatin1String( "Sample.txt" ) );
        const QString fileContents =  QLatin1String( "Some simple contents" ) ;

        FETCH_UPDATE_OPERATION(AppendFile, QStringList() << fileName << fileContents );
        bool success = oper->performOperation();
        printError(oper);
        QVERIFY2(success, "AppendFile could not append to an non-existing file");

        QString fileContents2;
        readFile(fileName, fileContents2);
        QVERIFY2(fileContents==fileContents2, "AppendFile did not write contents into the file properly");
    }

    // Case #4: Valid append into an existing file
    {
        const QString fileName = testDir.absoluteFilePath( QLatin1String( "Months.txt" ) );
        const QString fileContents1 =  QLatin1String( "Jan Feb Mar Apr May Jun" ) ;
        const QString fileContents2 =  QLatin1String( "Jul Aug Sep Oct Nov Dec" ) ;
        const QString fileContents3 = fileContents1 + fileContents2;

        createFile(fileName, fileContents1);

        FETCH_UPDATE_OPERATION(AppendFile, QStringList() << fileName << fileContents2 );
        bool success = oper->performOperation();
        printError(oper);
        QVERIFY2(success, "AppendFile could not append to an existing file");

        QString fcStr;
        readFile(fileName, fcStr);
        QVERIFY2(fcStr==fileContents3, "AppendFile did not append properly");
    }
}

void UpdateOperationsTest::testPrependFile()
{
    const QDir testDir = testDirectory( QLatin1String( "PrependFile" ) );

    // Case #1: Iinsufficient args
    {
        FETCH_UPDATE_OPERATION(PrependFile, QStringList());
        bool success = oper->performOperation();
        printError(oper);
        QVERIFY2(!success, "PrependFile succeeded with insufficient arguments");
    }

    // Case #2: Iinsufficient args
    {
        FETCH_UPDATE_OPERATION(PrependFile, QStringList() << testDir.absoluteFilePath( QLatin1String( "Sample.txt" ) ));
        bool success = oper->performOperation();
        printError(oper);
        QVERIFY2(!success, "PrependFile succeeded with insufficient arguments");
    }

    // Case #3: Valid prepend into a new file
    {
        const QString fileName = testDir.absoluteFilePath( QLatin1String( "Sample.txt" ) );
        const QString fileContents =  QLatin1String( "Some simple contents" ) ;

        FETCH_UPDATE_OPERATION(PrependFile, QStringList() << fileName << fileContents );
        bool success = oper->performOperation();
        printError(oper);
        QVERIFY2(success, "PrependFile could not prepend to an non-existing file");

        QString fileContents2;
        readFile(fileName, fileContents2);
        QVERIFY2(fileContents==fileContents2, "PrependFile did not write contents into the file properly");
    }

    // Case #4: Valid prepend into an existing file
    {
        const QString fileName = testDir.absoluteFilePath( QLatin1String( "Months.txt" ) );
        const QString fileContents1 =  QLatin1String( "Jan Feb Mar Apr May Jun" ) ;
        const QString fileContents2 =  QLatin1String( "Jul Aug Sep Oct Nov Dec" ) ;
        const QString fileContents3 = fileContents2 + fileContents1;

        createFile(fileName, fileContents1);

        FETCH_UPDATE_OPERATION(PrependFile, QStringList() << fileName << fileContents2 );
        bool success = oper->performOperation();
        printError(oper);
        QVERIFY2(success, "PrependFile could not prepend to an existing file");

        QString fcStr;
        readFile(fileName, fcStr);
        qDebug() << fcStr;
        qDebug() << fileContents3;
        QVERIFY2(fcStr==fileContents3, "PrependFile did not prepend properly");
    }
}

void UpdateOperationsTest::testExecute()
{
    const QDir testDir = testDirectory( QLatin1String( "Execute" ) );

    // Case #1: Pass insufficient arguments.
    {
        FETCH_UPDATE_OPERATION(Execute, QStringList());
        bool success = oper->performOperation();
        printError(oper);
        QVERIFY2(!success, "Execute succeeded, when when no params were given");
    }

    // Case #2: Execute a non-existing program
    {
        const QString program =  QLatin1String( "nonexistingprogram" ) ; // Lets hope this doesn't exist
        FETCH_UPDATE_OPERATION(Execute, QStringList() << program );
        bool success = oper->performOperation();
        printError(oper);
        QVERIFY2(!success, "Execute succeeded in executing a program that does not exist");
    }

    // Case #3: Execute some program
    {
#ifdef Q_WS_WIN
        const QString outputFile = testDir.absoluteFilePath( QLatin1String( "ipconfig.txt" ) );
        const QString program = QString::fromLatin1("ipconfig > %1").arg(outputFile);
#else
        const QString outputFile = testDir.absoluteFilePath( QLatin1String( "uname.txt" ) );
        const QString program = QString::fromLatin1("uname -a > %1").arg(outputFile);
#endif

#ifndef Q_WS_WIN
        FETCH_UPDATE_OPERATION(Execute, QStringList() << QLatin1String( "/bin/sh" ) << QLatin1String( "-c" ) << program );
#else
        FETCH_UPDATE_OPERATION(Execute, QStringList() << QLatin1String( "cmd.exe" ) << QLatin1String( "/C" ) << program );
#endif
        bool success = oper->performOperation();
        printError(oper);
        QVERIFY2(success, "Could not execute program");

        QString output;
        success = readFile( outputFile, output );
        QVERIFY2(success, "Program was not executed properly");

        // Lets run the program using QProcess and compare the
        // outputs.
        QString programOutput;
#ifdef Q_WS_WIN
        success = executeProgram( QLatin1String( "ipconfig" ) , programOutput);
#else
        success = executeProgram( QLatin1String( "uname -a" ) , programOutput);
#endif

        QVERIFY2(success, "Execute operation executed a program that cannot be executed");
        QVERIFY2(output==programOutput, "Execute operation did not execute the program correctly");
    }
}

void UpdateOperationsTest::testXml()
{
    QDomDocument xml;

    {
        PseudoUpdateOperation oper;
        oper.setValue( QLatin1String("foobar"), 3 );
        QVERIFY( oper.hasValue( QLatin1String("foobar") ) );
        QVERIFY( oper.value( QLatin1String("foobar") ) == 3 );
        oper.clearValue( QLatin1String("foobar") );
        QVERIFY( !oper.hasValue( QLatin1String("foobar") ) );
        oper.setValue( QLatin1String("foobar"), 3 );
        oper.setValue( QLatin1String("test"), qVariantFromValue< QPointF >( QPointF( 3.4, 2.1 ) ) );
        oper.setArguments( QStringList() << QLatin1String("foo") << QLatin1String("bar") );
        xml = oper.toXml();
    }

    PseudoUpdateOperation oper;
    QVERIFY( !oper.hasValue( QLatin1String("foobar") ) );
    QVERIFY( oper.fromXml( xml ) );
    QVERIFY( oper.hasValue( QLatin1String("foobar") ) );
    QVERIFY( oper.value( QLatin1String("foobar") ) == 3 );
    QVERIFY( oper.value( QLatin1String("test") ) == qVariantFromValue( QPointF( 3.4, 2.1 ) ) );
    QVERIFY( oper.arguments() == QStringList() << QLatin1String("foo") << QLatin1String("bar") );

    PseudoUpdateOperation* const cloned = oper.clone();
    QVERIFY( cloned->hasValue( QLatin1String("foobar") ) );
    QVERIFY( cloned->value( QLatin1String("foobar") ) == 3 );
    QVERIFY( cloned->value( QLatin1String("test") ) == qVariantFromValue( QPointF( 3.4, 2.1 ) ) );
    QVERIFY( cloned->arguments() == QStringList() << QLatin1String("foo") << QLatin1String("bar") );
    delete cloned;
}

QDir UpdateOperationsTest::testDirectory(const QString& operType)
{
    QDir dir = m_tempDirectory;
    dir.mkdir(operType);
    dir.cd(operType);
    return dir;
}

bool UpdateOperationsTest::createFile(const QString& fileName, const QString& contents)
{
    QFile file(fileName);
    if(!file.open(QFile::WriteOnly))
        return false;

    file.write( contents.toAscii() );
    return true;
}

bool UpdateOperationsTest::readFile(const QString& fileName, QString& contents)
{
    if( !QFile::exists(fileName) )
        return false;

    QFile file(fileName);
    if(!file.open(QFile::ReadOnly))
        return false;

    contents = QTextStream( &file ).readAll();
    return true;
}

void UpdateOperationsTest::printError(KDUpdater::UpdateOperation* oper)
{
    if(!oper)
        return;

    QString errorStr = oper->lastError();
    if(errorStr.isEmpty())
        return;

    qDebug("Error : %s", qPrintable(errorStr));
}

bool UpdateOperationsTest::executeProgram(const QString& program, QString& output)
{
    QProcess process;
    process.start(program);
    process.waitForStarted();
    if(process.error() == QProcess::FailedToStart)
        return false;

    process.waitForFinished();
    // How to check if the process is actually finished?

    output = QString::fromLocal8Bit( process.readAllStandardOutput().constData() );

    return true;
}

QTEST_MAIN(UpdateOperationsTest)

#include "main.moc"
