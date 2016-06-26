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

#include "kdupdaterpackagesinfo.h"
#include "kdupdatertarget.h"

#include <QFileInfo>
#include <QDomDocument>
#include <QDomElement>
#include <QVector>

#include <KDToolsCore/KDSaveFile>

/*!
   \ingroup kdupdater
   \class KDUpdater::PackagesInfo kdupdaterpackagesinfo.h KDUpdaterPackagesInfo
   \brief Provides access to information about packages installed on the application side.

   This class parses the XML package file specified via the setFileName() method and
   provides access to the information defined within the package file through an
   easy to use API. You can:
   \li get target name via the targetName property.
   \li get target version via the targetVersion property.
   \li get information about the number of packages installed and their meta-data via the
       packageInfoCount() and packageInfo() methods.

   Instances of this class cannot be created. Each instance of \ref KDUpdater::Target
   has one instance of this class associated with it. You can fetch a pointer to an instance
   of this class for a target via the \ref KDUpdater::Target::packagesInfo()
   method.
*/

/*! \enum UpdatePackagesInfo::Error
 * Error codes related to retrieving update sources
 */

/*! \var UpdatePackagesInfo::Error UpdatePackagesInfo::NoError
 * No error occurred
 */

/*! \var UpdatePackagesInfo::Error UpdatePackagesInfo::NotYetReadError
 * The package information was not parsed yet from the XML file
 */

/*! \var UpdatePackagesInfo::Error UpdatePackagesInfo::CouldNotReadPackageFileError
 * the specified update source file could not be read (does not exist or not readable)
 */

/*! \var UpdatePackagesInfo::Error UpdatePackagesInfo::InvalidXmlError
 * The source file contains invalid XML.
 */

/*! \var UpdatePackagesInfo::Error UpdatePackagesInfo::InvalidContentError
 * The source file contains valid XML, but does not match the expected format for package descriptions
 */

/*!
   \property KDUpdater::PackagesInfo::fileName

   Specifies the complete name of the Packages.xml file that this class referred to.
   Setting this propery also issus a call to \ref refresh() to reload package information
   from the XML file. The default value is an empty string.

   Get this property's value using %fileName(), and set it
   using %setFileName().

   \sa KDUpdater::Target::packagesXMLFileName
*/

/*!
   \property KDUpdater::PackagesInfo::targetName

   Specifies the target name. By default this is the name specified in
   the TargetName XML element of the Packages.xml file.

   Get this property's value using %targetName(), and set it
   using %setTargetName().
*/

/*!
   \property KDUpdater::PackagesInfo::targetVersion

   Specifies the target version. By default this is the version specified in
   the TargetVersion XML element of Packages.xml.

   Get this property's value using %targetVersion(), and set it
   using %setTargetVersion().
*/

/*!
   \property KDUpdater::PackagesInfo::compatLevel

   Specifies the compat level. By default this is the compat level specified in
   the CompatLevel XML element of Packages.xml.

   Get this property's value using %compatLevel(), and set it
   using %setCompatLevel().
*/

/*!
  \property KDUpdater::PackagesInfo::errorString

   Contains a human-readable error message.

   Get this property's value using %errorString().
*/

using namespace KDUpdater;

class PackagesInfo::Private
{
public:
    Private( PackagesInfo* qq, Target* app ) :
        q( qq ),
        target( app ),
        error(PackagesInfo::NotYetReadError),
        compatLevel( -1 )
    {
    }

private:
    PackagesInfo* q;

public:
    Target* const target;
    QString errorMessage;
    PackagesInfo::Error error;
    QString fileName;
    QString targetName;
    QString targetVersion;
    int compatLevel;
    bool modified;

    QVector<PackageInfo> packageInfoList;

    void addPackageFrom(const QDomElement& packageE);
    void setInvalidContentError( const QString& detail );
};

void PackagesInfo::Private::setInvalidContentError(const QString& detail)
{
    error = PackagesInfo::InvalidContentError;
    errorMessage = tr("%1 contains invalid content: %2").arg(fileName, detail);
}

/*!
   \internal
*/
PackagesInfo::PackagesInfo(Target* target)
    : d( new Private( this, target ) )
{
}

/*!
   \internal
*/
PackagesInfo::~PackagesInfo()
{
    if( d->modified && ( !d->packageInfoList.isEmpty() || QFile::exists( d->fileName ) ) )
        writeToDisk();
}

/*!
   Returns a pointer to the target, whose package information this class provides
   access to.
*/
Target* PackagesInfo::target() const
{
    return d->target;
}

/*!
   Returns \c true if the PackagesInfo are valid.  Otherwise, \c false
   is returned, in which case the errorString() method can be used to
   retrieve a descriptive error message.
*/
bool PackagesInfo::isValid() const
{
    return d->error == NoError;
}

QString PackagesInfo::errorString() const
{
    return d->errorMessage;
}

PackagesInfo::Error PackagesInfo::error() const
{
    return d->error;
}

void PackagesInfo::setFileName(const QString& fileName)
{
    if( d->fileName == fileName )
        return;

    d->fileName = fileName;
    refresh();
}

QString PackagesInfo::fileName() const
{
    return d->fileName;
}

void PackagesInfo::setTargetName(const QString& name)
{
    d->targetName = name;
    d->modified = true;
}

QString PackagesInfo::targetName() const
{
    return d->targetName;
}

void PackagesInfo::setTargetVersion(const QString& version)
{
    d->targetVersion = version;
    d->modified = true;
}

QString PackagesInfo::targetVersion() const
{
    return d->targetVersion;
}

/*!
   Returns the number of \ref KDUpdater::PackageInfo objects contained in this class.
*/
int PackagesInfo::packageInfoCount() const
{
    return d->packageInfoList.count();
}

/*!
   Returns the package info structure (\ref KDUpdater::PackageInfo) at \a index. If \a index is
   out of range then an empty package info structure is returned.
*/
PackageInfo PackagesInfo::packageInfo(int index) const
{
    if( index < 0 || index >= d->packageInfoList.count() )
        return PackageInfo();

    return d->packageInfoList[index];
}

int PackagesInfo::compatLevel() const
{
    return d->compatLevel;
}



/*!
   This function returns the index of the package whose name is \a pkgName. If no such
   package was found, this function returns -1.
*/
int PackagesInfo::findPackageInfo(const QString& pkgName) const
{
    int i = 0;
    for( QVector< PackageInfo >::const_iterator it = d->packageInfoList.begin(); it != d->packageInfoList.end(); ++it, ++i )
    {
        if( it->name == pkgName )
            return i;
    }

    return -1;
}

/*!
   Returns all package info structures.
*/
QVector<PackageInfo> PackagesInfo::packageInfos() const
{
    return d->packageInfoList;
}

/*!
   This function re-reads the Packages.xml file and updates itself. Changes to \ref targetName
   and \ref targetVersion are lost after this function returns. The function emits a reset()
   signal after completion.
*/
void PackagesInfo::refresh()
{
    // First clear internal variables
    d->targetName.clear();
    d->targetVersion.clear();
    d->packageInfoList.clear();
    d->modified = false;

    QFile file( d->fileName );

    // if the file does not exist then we just skip the reading
    if( !file.exists() )
    {
        d->error = NoError;
        d->errorMessage.clear();
        emit reset();
        return;
    }

    // Open Packages.xml
    if( !file.open(QFile::ReadOnly) )
    {
        d->error = CouldNotReadPackageFileError;
        d->errorMessage = tr("Could not read \"%1\"").arg(d->fileName);
        emit reset();
        return;
    }

    // Parse the XML document
    QDomDocument doc;
    QString parseErrorMessage;
    int parseErrorLine;
    int parseErrorColumn;
    if( !doc.setContent( &file, &parseErrorMessage, &parseErrorLine, &parseErrorColumn ) )
    {
        d->error = InvalidXmlError;
        d->errorMessage = tr("Parse error in %1 at %2, %3: %4")
                          .arg(d->fileName,
                               QString::number(parseErrorLine),
                               QString::number(parseErrorColumn),
                               parseErrorMessage);
        emit reset();
        return;
    }
    file.close();

    // Now populate information from the XML file.
    const QDomElement rootE = doc.documentElement();
    if( rootE.tagName() != QLatin1String( "Packages" ) )
    {
        d->setInvalidContentError(tr("root element %1 unexpected, should be \"Packages\"").arg(rootE.tagName()));
        emit reset();
        return;
    }

    for( QDomNode childNode = rootE.firstChild(); !childNode.isNull(); childNode = childNode.nextSiblingElement() )
    {
        const QDomElement childNodeE = childNode.toElement();
        if( childNodeE.isNull() )
            continue;

        if ( childNodeE.tagName() == QLatin1String( "TargetName" ) ||
             childNodeE.tagName() == QLatin1String( "ApplicationName" ) ) // backwards compat
            d->targetName = childNodeE.text();
        else if ( childNodeE.tagName() == QLatin1String( "TargetVersion" ) ||
                  childNodeE.tagName() == QLatin1String( "ApplicationVersion" ) )
            d->targetVersion = childNodeE.text();
        else if( childNodeE.tagName() == QLatin1String( "Package" ) )
            d->addPackageFrom( childNodeE );
        else if( childNodeE.tagName() == QLatin1String( "CompatLevel" ) )
            d->compatLevel = childNodeE.text().toInt();
    }

    d->error = NoError;
    d->errorMessage.clear();
    emit reset();
}

void PackagesInfo::setCompatLevel(int level)
{
    d->compatLevel = level;
    d->modified = true;
}

/*!
 Marks the package with \a name as installed in \a version.
 */
bool PackagesInfo::installPackage( const QString& name, const QString& version, const QString& title, const QString& description
                                 , const QStringList& dependencies, quint64 uncompressedSize )
{
    if( findPackageInfo( name ) != -1 )
        return updatePackage( name, version, QDate::currentDate() );

    PackageInfo info;
    info.name = name;
    info.version = version;
    info.installDate = QDate::currentDate();
    info.title = title;
    info.description = description;
    info.dependencies = dependencies;
    info.uncompressedSize = uncompressedSize;
    d->packageInfoList.push_back( info );
    d->modified = true;
    return true;
}

/*!
   #Update the package.
*/
bool PackagesInfo::updatePackage(const QString &name,
                                            const QString &version,
                                            const QDate &date)
{
    const int index = findPackageInfo(name);

    if (index==-1) return false;

    d->packageInfoList[index].version = version;
    d->packageInfoList[index].lastUpdateDate = date;
    d->modified = true;
    return true;
}

/*!
 Remove the package with \a name.
 */
bool PackagesInfo::removePackage( const QString& name )
{
    const int index = findPackageInfo( name );
    if( index == -1 )
        return false;

    d->packageInfoList.remove( index );
    d->modified = true;
    return true;
}

static void addTextChildHelper(QDomNode *node,
                               const QString &tag,
                               const QString &text)
{
    QDomElement domElement = node->ownerDocument().createElement(tag);
    const QDomText domText = node->ownerDocument().createTextNode(text);

    domElement.appendChild(domText);
    node->appendChild(domElement);
}

void PackagesInfo::writeToDisk()
{
    QDomDocument doc;
    QDomElement root = doc.createElement(QLatin1String( "Packages") ) ;
    doc.appendChild(root);

    addTextChildHelper(&root, QLatin1String( "TargetName" ), d->targetName);
    addTextChildHelper(&root, QLatin1String( "TargetVersion" ), d->targetVersion);
    if (d->compatLevel!=-1) {
        addTextChildHelper(&root, QLatin1String( "CompatLevel" ), QString::number(d->compatLevel));
    }

    Q_FOREACH (const PackageInfo &info, d->packageInfoList) {
        QDomElement package = doc.createElement( QLatin1String( "Package" ) );

        addTextChildHelper( &package, QLatin1String( "Name" ), info.name );
        addTextChildHelper( &package, QLatin1String( "Pixmap" ), info.pixmap );
        addTextChildHelper( &package, QLatin1String( "Title" ), info.title );
        addTextChildHelper( &package, QLatin1String( "Description" ), info.description );
        addTextChildHelper( &package, QLatin1String( "Version" ), info.version );
        addTextChildHelper( &package, QLatin1String( "LastUpdateDate" ), info.lastUpdateDate.toString( Qt::ISODate ) );
        addTextChildHelper( &package, QLatin1String( "InstallDate" ), info.installDate.toString( Qt::ISODate) );
        addTextChildHelper( &package, QLatin1String( "Size" ), QString::number( info.uncompressedSize ) );
        QString assembledDependencies = QLatin1String( "" );
        Q_FOREACH( const QString & val, info.dependencies ){
            assembledDependencies += val + QLatin1String( "," );
        }
        if ( info.dependencies.count() > 0 )
            assembledDependencies.chop( 1 );
        addTextChildHelper( &package, QLatin1String( "Dependencies" ), assembledDependencies );

        root.appendChild(package);
    }

    // Open Packages.xml
    KDSaveFile file( d->fileName );
    if( !file.open(QFile::WriteOnly) ) {
        return;
    }

    file.write(doc.toByteArray(4));
    file.commit( KDSaveFile::OverwriteExistingFile );
}

void PackagesInfo::Private::addPackageFrom(const QDomElement& packageE)
{
    if( !packageE.hasChildNodes() )
        return;

    PackageInfo info;
    for( QDomNode childNode = packageE.firstChild(); !childNode.isNull(); childNode = childNode.nextSibling() )
    {
        const QDomElement childNodeE = childNode.toElement();
        if( childNodeE.isNull() )
            continue;

        if( childNodeE.tagName() == QLatin1String( "Name" ) )
            info.name = childNodeE.text();
        else if( childNodeE.tagName() == QLatin1String( "Pixmap" ) )
            info.pixmap = childNodeE.text();
        else if( childNodeE.tagName() == QLatin1String( "Title" ) )
            info.title = childNodeE.text();
        else if( childNodeE.tagName() == QLatin1String( "Description" ) )
            info.description = childNodeE.text();
        else if( childNodeE.tagName() == QLatin1String( "Version" ) )
            info.version = childNodeE.text();
        else if( childNodeE.tagName() == QLatin1String( "Size" ) )
            info.uncompressedSize = childNodeE.text().toULongLong();
        else if( childNodeE.tagName() == QLatin1String( "Dependencies" ) )
            info.dependencies = childNodeE.text().split( QLatin1String( "," ), QString::SkipEmptyParts );
        else if( childNodeE.tagName() == QLatin1String( "LastUpdateDate" ) )
            info.lastUpdateDate = QDate::fromString(childNodeE.text(), Qt::ISODate);
        else if( childNodeE.tagName() == QLatin1String( "InstallDate" ) )
            info.installDate = QDate::fromString(childNodeE.text(), Qt::ISODate);
    }

    this->packageInfoList.append( info );
}

/*!
   \fn KDUpdater::PackagesInfo::reset()

   This signal is emitted whenever the contents of this class is refreshed, usually from within
   the \ref refresh() slot.
*/

/*!
   \ingroup kdupdater
   \struct KDUpdater::PackageInfo kdupdaterpackagesinfo.h KDUpdaterPackageInfo
   \brief Describes a single installed package in the target.

   This structure contains information about a single installed package in the target.
   The information contained in this structure corresponds to the information described
   by the Package XML element in Packages.xml
*/

/*!
   \var QString KDUpdater::PackageInfo::name

   The name of the package.
*/

/*!
   \var QString KDUpdater::PackageInfo::pixmap

   An icon for the package.
*/

/*!
   \var QString KDUpdater::PackageInfo::title

   The human-readable name (#title) of the package.
*/

/*!
   \var QString KDUpdater::PackageInfo::description

   A descriptive text for the package.
*/

/*!
   \var QString KDUpdater::PackageInfo::version

   The package version.
*/

/*!
   \var QDate KDUpdater::PackageInfo::lastUpdateDate

   The date the package was last updated.
*/

/*!
   \var QDate KDUpdater::PackageInfo::installDate

   The date the package was installed.
*/
