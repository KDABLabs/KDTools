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

#include "kdupdatertarget.h"
#include "kdupdaterpackagesinfo.h"
#include "kdupdaterupdatesourcesinfo.h"
#include "kdupdatertask.h"

#include <QCoreApplication>
#include <QDir>

using namespace KDUpdater;

/*!
   \defgroup kdupdater KD Updater
   \since_l 2.1

KDUpdater is a tool to automatically detect, retrieve, install and activate
updates to software applications and libraries. It is intended to be used with
Qt based applications, and developed against the Qt 4 series. It is a library
that users link to their application. It uses only accepted standard protocols,
and does not require any other 3rd party libraries that are not shipped with Qt.
KDUpdater is available on Linux, Mac Os X and Windows.

KDUpdater implements the necessary platform dependent ways of deploying
applications and shields the application developers from these subtleties. It is
implemented in C++ using Qt 4.

\section kdupdater_targetaudience Target Audience

The users of KDUpdater are Qt programmers. Indirectly, end users will experience
the GUI elements of it.

\section kdupdater_keyfeatures Key Features

KDUpdater provides the following major features:

\li It is a programmer's tool. Programmers integrate it into their application,
and define the actions taken in case there is a new software version.
KDUpdater identifies new software versions, and is able to discriminate
different target platforms and architectures. It notifies the application of
available packages.

\li KDUpdater would be a module within the client application. This means that
the KDUpdater module would not be useful as an independent entity. It has to be
used within a Qt application.

\li KDUpdater is able to download, install and activate update packages from
FTP and HTTP server using standard compliant protocols. No specific server
software is necessary. The packages are selected by the file system structure on
the server and a naming convention.

\li KDUpdater is capable of determining the applicable upgrade packages out of
a provided set of packages that target different deployment platforms of the
same software.

\li KDUpdater keeps the previously activated version, and is capable of rolling
back the update on request.

\li KDUpdater is a tool to implement GUI based automatic updates. It does not
interface automatic package management systems like apt, rpm or ports.

\li KDUpdater provides basic security features like checking the signature of a
package list against a public key, logging into the update server using
encrypted connections, and authenticating using a challenge-response protocol.
It should be generally safe to operate KDUpdater over the public Internet.

\section kdupdater_designconsiderations Design Considerations

KDUpdater is based on Qt 4.3 or later. It requires no 3rd party libraries to be
installed or linked other than the ones already linked into Qt 4. It is written
in standard conforming C++, and compiles with the native development toolkits on
the target platforms. It uses QMake as provided with the used Qt library to
build.

KDUpdater assumes an Internet connection that allows the creation of SSL/TLS
encrypted HTTP connections, or of comparably secured FTP connections. It assumes
that network connections are unstable, and may be interrupted. In that case, it
will recover from an interrupted transfer.

KDUpdater does not require a GUI session, but defaults to expecting one. Users
can program KDUpdater to work for command line based applications.

\section kdupdater_furtherreading Further Reading

More information about the KDUpdater architecture can be found in the following
pages

\li \subpage kdupdater_architecture
\li \subpage kdupdater_updateserverarch
\li \subpage kdupdater_updateclientarch
\li \subpage kdupdater_xmlfileformats
\li \subpage kdupdater_updatefileformat
*/

/*!
   \namespace KDUpdater
*/

/*!
   \class KDUpdater::Target kdupdatertarget.h KDUpdaterTarget
   \ingroup kdupdater
   \brief This class represents a target (application) that can be updated.

   A KDUpdater target is an application that needs to interact with one or more update servers and
   downloads/installs updates This class helps in describing a target in terms of:
   \li target Directory
   \li packages XML file name and its corresponding KDUpdater::PackagesInfo object
   \li update Sources XML file name and its corresponding KDUpdater::UpdateSourcesInfo object

   User can also retrieve some information from this class:
   \li target name
   \li target version
   \li compat level
*/

/*!
   \property KDUpdater::Target::directory

   Specifies the path to the target directory containing UpdateSources.xml and Packages.xml.
   The default value is QCoreApplication::applicationDirPath().

   If you manually set this to another value, Target does not check whether the directory exists.

   Get this property's value using %directory(), and set it
   using %setDirectory().
*/

/*!
   \property KDUpdater::Target::name

   Contains the target name. The default is
   QCoreApplication::applicationName().

   Get this property's value using %name(), and set it using the \link
   Target::Target(const QString&) constructor\endlink.
*/

/*!
   \property KDUpdater::Target::version

   Contains the target version.

   Get this property's value using %version().
*/

/*!
   \property KDUpdater::Target::compatLevel

   Contains the compat level that this target is in.

   Get this property's value using %compatLevel().
*/

/*!
   \property KDUpdater::Target::packagesXMLFileName

   Specifies the file name of the Package XML file for this target. By default this is assumed to be
   \c Packages.xml in the \c application directory.

   Get this property's value using %packagesXMLFileName(), and set it
   using %setPackagesXMLFileName().

   \sa KDUpdater::PackagesInfo::fileName
*/

/*!
   \property KDUpdater::Target::updateSourcesXMLFileName

   Specifies the file name of the Package XML file for this target. By default this is assumed to be
   \c Packages.xml in the \c application directory.

   Get this propery's value using %updateSourcesXMLFileName(), and set it
   using %setUpdateSourcesXMLFileName().

   \sa KDUpdater::UpdateSourcesInfo::fileName
*/

class Target::Private
{
public:
    Private( Target* qq, const QString & n, PackagesInfo* pinfo, UpdateSourcesInfo* usinfo )
        : q( qq ),
          name( n ),
          packagesInfo( pinfo ),
          updateSourcesInfo( usinfo )
    {
    }

private:
    Target* const q;

public:
    QString name, directory;
    PackagesInfo* const packagesInfo;
    UpdateSourcesInfo* const updateSourcesInfo;
    QObject taskOwner;
};

/*!
   Constructor of the Target class. The class will be constructed and configured to
   assume the target directory to be the directory in which the application exists. The
   target name is assumed to be QCoreApplication::applicationName()
*/
Target::Target()
    : d( new Private( this,
                      QCoreApplication::applicationName(),
                      new PackagesInfo( this ),
                      new UpdateSourcesInfo( this ) ) )
{
    setDirectory( QCoreApplication::applicationDirPath() );
}

/*!
   Constructor of the Target class. The class will be constructed and configured to
   assume the target directory to be the directory in which the application exists. The
   target name is set to \a name.
*/
Target::Target( const QString & name )
    : d( new Private( this,
                      name,
                      new PackagesInfo( this ),
                      new UpdateSourcesInfo( this ) ) )
{
    setDirectory( QCoreApplication::applicationDirPath() );
}

/*!
   Destructor.
*/
Target::~Target()
{
    delete d->packagesInfo;
    delete d->updateSourcesInfo;
}

void Target::addTask( Task * task )
{
    if ( task )
        task->setParent( &d->taskOwner );
}

void Target::setDirectory( const QString & dir )
{
    if( d->directory == dir )
        return;

    const QDir dirObj(dir);

    d->directory = dirObj.absolutePath();
    setPackagesXMLFileName( dirObj.absoluteFilePath( QLatin1String( "Packages.xml" ) ) );
    setUpdateSourcesXMLFileName( dirObj.absoluteFilePath( QLatin1String( "UpdateSources.xml" ) ) );
}

QString Target::directory() const
{
    return d->directory;
}

QString Target::name() const
{
    if( d->packagesInfo->isValid() )
        return d->packagesInfo->targetName();

    return d->name;
}

QString Target::version() const
{
    if( d->packagesInfo->isValid() )
        return d->packagesInfo->targetVersion();

    return QString();
}

int Target::compatLevel() const
{
    if(d->packagesInfo->isValid())
        return d->packagesInfo->compatLevel();

    return -1;
}

/*!
   Adds an update source named \a name to the target. The parameters are the same as in UpdateSources.xml, i.e.
   \a name is the source's name, \a title is a human readable title for the update source, \a description is
   a human readable description, \a url the URL of the update source containing Updates.xml and \a priority the
   priority of the update source. A lower number means a higher priority.
*/
void Target::addUpdateSource( const QString& name, const QString& title,
                                              const QString& description, const QUrl& url, int priority )
{
    UpdateSourceInfo info;
    info.name = name;
    info.title = title;
    info.description = description;
    info.url = url;
    info.priority = priority;
    d->updateSourcesInfo->addUpdateSourceInfo( info );
}

void Target::setPackagesXMLFileName(const QString& fileName)
{
    d->packagesInfo->setFileName( fileName );
}

QString Target::packagesXMLFileName() const
{
    return d->packagesInfo->fileName();
}

/*!
   Returns the \ref PackagesInfo object associated with this target.
*/
PackagesInfo* Target::packagesInfo() const
{
    return d->packagesInfo;
}

void Target::setUpdateSourcesXMLFileName(const QString& fileName)
{
    d->updateSourcesInfo->setFileName( fileName );
}

QString Target::updateSourcesXMLFileName() const
{
    return d->updateSourcesInfo->fileName();
}

/*!
   Returns the \ref UpdateSourcesInfo object associated with this target.
*/
UpdateSourcesInfo* Target::updateSourcesInfo() const
{
    return d->updateSourcesInfo;
}
