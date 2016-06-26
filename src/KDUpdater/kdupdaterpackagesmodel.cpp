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

#include "kdupdaterpackagesmodel.h"
#include "kdupdaterpackagesinfo.h"

/*!
   \ingroup kdupdater
   \class KDUpdater::PackagesModel kdupdaterpackagesmodel.h KDUpdaterPackagesModel
   \brief A model that can show packages contained in \ref KDUpdater::PackagesInfo

   \ref KDUpdater::PackagesInfo, associated with \ref KDUpdater::Target, contains
   information about all the packages installed in the application. This model helps view the packages
   in a list.

   \image html packagesview.jpg

   To use this model, just create an instance and pass to \ref setPackageInfo() a pointer to
   \ref KDUpdater::PackagesInfo whose information you want this model to contain.
*/

using namespace KDUpdater;

class PackagesModel::Private
{
public:
    Private( PackagesModel* qq )
        : q( qq ),
          packagesInfo( 0 )
    {
    }

private:
    PackagesModel* const q;

public:
    void refresh();

    const PackagesInfo* packagesInfo;
};

/*!
   Constructor.
*/
PackagesModel::PackagesModel( QObject* parent )
    : QAbstractTableModel( parent ),
      d ( new Private( this ) )
{
}

/*!
   Destructor
*/
PackagesModel::~PackagesModel()
{
}

/*!
   Sets the package info whose information this model should contain.

   \code
   KDUpdater::Target target;

   KDUpdater::PackagesModel packageModel;
   packageModel.setPackageInfo( target.packagesInfo() );
   
   QTreeView view;
   view.setModel( &packageModel );
   view.show();
   \endcode

*/
void PackagesModel::setPackageInfo( const PackagesInfo* packagesInfo )
{
    if( d->packagesInfo == packagesInfo )
        return;

    if( d->packagesInfo )
        disconnect( d->packagesInfo, 0, this, 0 );

    d->packagesInfo = packagesInfo;
    if( d->packagesInfo )
        connect( d->packagesInfo, SIGNAL(reset()), this, SLOT(refresh()) );

    d->refresh();
}

/*!
   Returns a pointer to the package info whose information this model is containing.
*/
const PackagesInfo* PackagesModel::packagesInfo() const
{
    return d->packagesInfo;
}

/*!
 \reimp
*/
int PackagesModel::rowCount( const QModelIndex& parent ) const
{
    if( d->packagesInfo == 0 )
        return 0;

    return parent.isValid() ? 0 : d->packagesInfo->packageInfoCount();
}

/*!
 \reimp
*/
int PackagesModel::columnCount( const QModelIndex& parent ) const
{
    return parent.isValid() ? 0 : 5;
}

/*!
 \reimp
*/
QVariant PackagesModel::data( const QModelIndex& index, int role ) const
{
    if( !index.isValid() || d->packagesInfo == 0 )
        return QVariant();

    if( role != Qt::DisplayRole )
        return QVariant();
    
    const PackageInfo& info = d->packagesInfo->packageInfo( index.row() );
    switch( index.column() )
    {
    case Name:
        return info.name;
    case Title:
        return info.title;
    case Description:
        return info.description;
    case Version:
        return info.version;
    case LastUpdate:
        return info.lastUpdateDate.toString();
    }
    return QVariant();
}

/*!
 \reimp
*/
QVariant PackagesModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    if( d->packagesInfo == 0 )
        return QAbstractTableModel::headerData( section, orientation, role );
    
    if( role != Qt::DisplayRole || orientation != Qt::Horizontal )
        return QAbstractTableModel::headerData( section, orientation, role );

    switch( section )
    {
    case Name:
        return tr( "Name" );
    case Title:
        return tr( "Title" );
    case Description:
        return tr( "Description" );
    case Version:
        return tr( "Version" );
    case LastUpdate:
        return tr( "Last Updated" );
    }

    return QAbstractTableModel::headerData( section, orientation, role );
}

/*!
   This slot reloads package information from the \ref KDUpdater::PackagesInfo associated
   with this model.

   \note By default, this slot is connected to the \ref KDUpdater::PackagesInfo::reset()
   signal in \ref setPackageInfo()
*/
void PackagesModel::Private::refresh()
{
    q->reset();
}

#include "moc_kdupdaterpackagesmodel.cpp"
