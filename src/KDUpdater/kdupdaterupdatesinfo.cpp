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

#include "kdupdaterupdatesinfo_p.h"

#include <QCoreApplication>
#include <QDomDocument>
#include <QDomElement>
#include <QFile>
#include <QSharedData>
#include <QUrl>

#include <algorithm>
#include <functional>

using namespace KDUpdater;

//
// KDUpdater::UpdatesInfo::Private
//
class UpdatesInfo::Private: public QSharedData
{
    Q_DECLARE_TR_FUNCTIONS(KDUpdater::Private)

public:
    Private() : error(UpdatesInfo::NotYetReadError), compatLevel(-1) { }

    QString errorMessage;
    UpdatesInfo::Error error;
    QString updateXmlFile;
    QString targetName;
    QString targetVersion;
    int compatLevel;
    QVector<UpdateInfo> updateInfoList;

    void parseFile(const QString& updateXmlFile);
    bool parsePackageUpdateElement(const QDomElement & updateE);
    bool parseCompatUpdateElement(const QDomElement & updateE);

    void setInvalidContentError( const QString& detail );
};

/*!
 Return true when the type of \a info is \a t.
 \internal
 */
struct TypeIs : public std::unary_function< UpdateInfo, bool >
{
    explicit TypeIs( int t )
        : type( t )
    {
    }

    bool operator()( const UpdateInfo& info ) const
    {
        return info.type == type;
    }

private:
    const int type;
};

/*!
 Returns true when the CompatLevel or the RequiredCompatLevel of \a info is not \a c.
 \internal
 */
struct CompatLevelIsNot : public std::unary_function< UpdateInfo, bool >
{
    explicit CompatLevelIsNot( int c )
        : compatLevel( c )
    {
    }

    bool operator()( const UpdateInfo& info ) const
    {
        if( info.type == CompatUpdate )
            return info.data.value( QLatin1String( "CompatLevel" ) ) != compatLevel;
        else
            return info.data.value( QLatin1String( "RequiredCompatLevel" ) ) != compatLevel;
    }
    
private:
    const QVariant compatLevel;
};

void UpdatesInfo::Private::setInvalidContentError(const QString& detail)
{
    error = UpdatesInfo::InvalidContentError;
    errorMessage = tr("Updates.Xml contains invalid content: %1").arg(detail);
}

void UpdatesInfo::Private::parseFile(const QString& updateXmlFile)
{
    QFile file( updateXmlFile );
    if( !file.open(QFile::ReadOnly) )
    {
        error = UpdatesInfo::CouldNotReadUpdateInfoFileError;
        errorMessage = tr("Could not read \"%1\"").arg(updateXmlFile);
        return;
    }

    QDomDocument doc;
    QString parseErrorMessage;
    int parseErrorLine;
    int parseErrorColumn;
    if( !doc.setContent( &file, &parseErrorMessage, &parseErrorLine, &parseErrorColumn ) )
    {
        error = UpdatesInfo::InvalidXmlError;
        errorMessage = tr("Parse error in %1 at %2, %3: %4")
                      .arg( updateXmlFile, 
                            QString::number( parseErrorLine ),
                            QString::number( parseErrorColumn ),
                            parseErrorMessage );
        return;
    }

    const QDomElement rootE = doc.documentElement();
    if( rootE.tagName() != QLatin1String( "Updates" ) )
    {
        setInvalidContentError(tr("root element %1 unexpected, should be \"Updates\"").arg(rootE.tagName()));
        return;
    }

    const QDomNodeList childNodes = rootE.childNodes();
    for(int i=0; i<childNodes.count(); i++)
    {
        const QDomNode childNode = childNodes.at(i);
        const QDomElement childE = childNode.toElement();
        if( childE.isNull() )
            continue;

        if ( childE.tagName() == QLatin1String( "TargetName" ) ||
             childE.tagName() == QLatin1String( "ApplicationName" ) ) // backwards compat
            targetName = childE.text();
        else if ( childE.tagName() == QLatin1String( "TargetVersion" ) ||
                  childE.tagName() == QLatin1String( "ApplicationVersion" ) ) // backwards compat
            targetVersion = childE.text();
        else if( childE.tagName() == QLatin1String( "RequiredCompatLevel" ) )
            compatLevel = childE.text().toInt();
        else if( childE.tagName() == QLatin1String( "PackageUpdate" ) ) {
            const bool res = parsePackageUpdateElement( childE );
            if (!res) {
                //error handled in subroutine
                return;
            }
        } else if( childE.tagName() == QLatin1String( "CompatUpdate" ) ) {
            const bool res = parseCompatUpdateElement( childE );
            if (!res) {
                //error handled in subroutine
                return;
            }
        }
    }

    if ( targetName.isEmpty() )
    {
        setInvalidContentError( tr("TargetName element is missing") );
        return;
    }
    
    if ( targetVersion.isEmpty() )
    {
        setInvalidContentError(tr("TargetVersion element is missing"));
        return;
    }
    
    error = UpdatesInfo::NoError;
    errorMessage.clear();
}

bool UpdatesInfo::Private::parsePackageUpdateElement(const QDomElement & updateE)
{
    if( updateE.isNull() )
        return false;

    UpdateInfo info;
    info.type = PackageUpdate;

    const QDomNodeList childNodes = updateE.childNodes();
    for(int i=0; i<childNodes.count(); i++)
    {
        const QDomNode childNode = childNodes.at(i);
        const QDomElement childE = childNode.toElement();
        if( childE.isNull() )
            continue;

        if( childE.tagName() == QLatin1String( "ReleaseNotes" ) ) {
            info.data[childE.tagName()] = QUrl(childE.text());
        }
        else if( childE.tagName() == QLatin1String( "UpdateFile" ) )
        {
            KDUpdater::UpdateFileInfo ufInfo;
            ufInfo.arch = childE.attribute(QLatin1String( "Arch" ), QLatin1String( "i386" ));
            ufInfo.platformRegEx = childE.attribute(QLatin1String( "platform-regex" ), QLatin1String( ".*" ));
            ufInfo.compressedSize = childE.attribute( QLatin1String( "CompressedSize" ) ).toLongLong();
            ufInfo.uncompressedSize = childE.attribute( QLatin1String( "UncompressedSize" ) ).toLongLong();
            ufInfo.sha1sum = QByteArray::fromHex( childE.attribute( QLatin1String( "sha1sum" ) ).toAscii() );
            ufInfo.fileName = childE.text();
            info.updateFiles.append(ufInfo);
        }
        else {
            info.data[childE.tagName()] = childE.text();
        }
    }

    if (!info.data.contains( QLatin1String( "Name" ) ))
    {
        setInvalidContentError(tr("PackageUpdate element without Name"));
        return false;
    }
    else if (!info.data.contains( QLatin1String( "Version" ) ))
    {
        setInvalidContentError(tr("PackageUpdate element without Version"));
        return false;
    }
    else if (!info.data.contains( QLatin1String( "ReleaseDate" ) ))
    {
        setInvalidContentError(tr("PackageUpdate element without ReleaseDate"));
        return false;
    }
    else if (info.updateFiles.isEmpty())
    {
        setInvalidContentError(tr("PackageUpdate element without UpdateFile"));
        return false;
    }

    updateInfoList.append(info);
    return true;
}

bool UpdatesInfo::Private::parseCompatUpdateElement(const QDomElement & updateE)
{
    if( updateE.isNull() )
        return false;

    UpdateInfo info;
    info.type = CompatUpdate;

    for( QDomNode childNode = updateE.firstChild(); !childNode.isNull(); childNode = childNode.nextSiblingElement() )
    {
        const QDomElement childE = childNode.toElement();
        if( childE.isNull() )
            continue;

        if( childE.tagName() == QLatin1String( "ReleaseNotes" ) ) {
            info.data[childE.tagName()] = QUrl(childE.text());
        }
        else if( childE.tagName() == QLatin1String( "UpdateFile" ) )
        {
            UpdateFileInfo ufInfo;
            ufInfo.platformRegEx = childE.attribute(QLatin1String( "platform-regex" ), QLatin1String( ".*" ));
            ufInfo.compressedSize = childE.attribute( QLatin1String( "compressed-size" ) ).toLongLong();
            ufInfo.uncompressedSize = childE.attribute( QLatin1String( "uncompressed-size" ) ).toLongLong();
            ufInfo.arch = childE.attribute(QLatin1String( "Arch" ), QLatin1String( "i386" ));
            ufInfo.fileName = childE.text();
            info.updateFiles.append(ufInfo);
        }
        else {
            info.data[childE.tagName()] = childE.text();
        }
    }

    if (!info.data.contains( QLatin1String( "CompatLevel" ) ))
    {
        setInvalidContentError(tr("CompatUpdate element without CompatLevel"));
        return false;
    }
    
    if (!info.data.contains( QLatin1String( "ReleaseDate" ) ))
    {
        setInvalidContentError(tr("CompatUpdate element without ReleaseDate"));
        return false;
    }
    
    if (info.updateFiles.isEmpty())
    {
        setInvalidContentError(tr("CompatUpdate element without UpdateFile"));
        return false;
    }

    updateInfoList.append(info);
    return true;
}


//
// KDUpdater::UpdatesInfo
//
UpdatesInfo::UpdatesInfo()
    : d( new UpdatesInfo::Private )
{
}

UpdatesInfo::~UpdatesInfo()
{
}

bool UpdatesInfo::isValid() const
{
    return d->error == NoError;
}

QString UpdatesInfo::errorString() const
{
    return d->errorMessage;
}

void UpdatesInfo::setFileName(const QString& updateXmlFile)
{
    if( d->updateXmlFile == updateXmlFile )
        return;

    d->targetName.clear();
    d->targetVersion.clear();
    d->updateInfoList.clear();

    d->updateXmlFile = updateXmlFile;
    d->parseFile( d->updateXmlFile );
}

QString UpdatesInfo::fileName() const
{
    return d->updateXmlFile;
}

QString UpdatesInfo::targetName() const
{
    return d->targetName;
}

QString UpdatesInfo::targetVersion() const
{
    return d->targetVersion;
}

int UpdatesInfo::compatLevel() const
{
    return d->compatLevel;
}

int UpdatesInfo::updateInfoCount( int type) const
{
    if ( type == AllUpdate )
        return d->updateInfoList.count();

    return static_cast< int >( std::count_if( d->updateInfoList.begin(), d->updateInfoList.end(), TypeIs( type ) ) );
}

UpdateInfo UpdatesInfo::updateInfo(int index) const
{
    if(index < 0 || index >= d->updateInfoList.count())
        return UpdateInfo();

    return d->updateInfoList.at( index );
}

QVector<UpdateInfo> UpdatesInfo::updatesInfo( int type,  int compatLevel ) const
{
    QVector<UpdateInfo> list;
    if ( compatLevel == -1 ) {
        if ( type == AllUpdate ) {
            return d->updateInfoList;
        }
        std::remove_copy_if( d->updateInfoList.begin(), d->updateInfoList.end(), std::back_inserter( list ), std::not1( TypeIs( type ) ) );
                }
                else {
        // first remove all wrong types
        std::remove_copy_if( d->updateInfoList.begin(), d->updateInfoList.end(), std::back_inserter( list ), std::not1( TypeIs( type ) ) );

        // the remove all wrong compat levels
        list.erase( 
            std::remove_if( list.begin(), list.end(), CompatLevelIsNot( compatLevel ) ), 
            list.end() );
    }
    return list;
}
