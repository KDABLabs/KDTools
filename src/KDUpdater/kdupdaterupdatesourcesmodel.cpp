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

#include "kdupdaterupdatesourcesmodel.h"
#include "kdupdaterupdatesourcesinfo.h"

/*!
   \ingroup kdupdater
   \class KDUpdater::UpdateSourcesModel kdupdaterupdatesourcesmodel.h KDUpdaterUpdateSourcesModel
   \brief A model that helps view and/or edit \ref KDUpdater::UpdateSourcesInfo

   \ref KDUpdater::UpdateSourcesInfo, associated with \ref KDUpdater::Target, contains information
   about all the update sources from which the application can download and install updates.
   This model helps view and edit update sources information.

   \image html updatesourcesview.jpg
*/

using namespace KDUpdater;

class UpdateSourcesModel::Private
{
public:
    Private( UpdateSourcesModel* qq ) 
        : q( qq ),
          updateSourcesInfo(0)
    {
    }

    void refresh();

    void slotAboutToAddUpdateSourceInfo( int index );
    void slotUpdateSourceInfoAdded();
    
    void slotAboutToRemoveUpdateSourceInfo( const UpdateSourceInfo& info );
    void slotUpdateSourceInfoRemoved();
    
    void slotUpdateSourceInfoChanged( const UpdateSourceInfo& newInfo );

private:
    UpdateSourcesModel* const q;

public:
    UpdateSourcesInfo* updateSourcesInfo;
};

/*!
   Constructor
*/
UpdateSourcesModel::UpdateSourcesModel( QObject* parent )
    : QAbstractTableModel( parent ),
      d( new Private( this ) )
{
}

/*!
   Destructor
*/
UpdateSourcesModel::~UpdateSourcesModel()
{
}

/*!
   Sets the \ref KDUpdater::UpdateSourcesInfo object whose information this model should contain.

   \code
   KDUpdater::Target target;

   KDUpdater::UpdateSourcesModel updatesModel;
   updatesModel.setUpdateSourcesInfo( target.updateSourcesInfo() );

   QTreeView view;
   view.setModel( &updatesModel );
   view.show();
   \endcode
*/
void UpdateSourcesModel::setUpdateSourcesInfo( UpdateSourcesInfo* info )
{
    if( d->updateSourcesInfo == info )
        return;

    if( d->updateSourcesInfo )
        disconnect( d->updateSourcesInfo, 0, this, 0 );

    d->updateSourcesInfo = info;
    if( d->updateSourcesInfo )
    {
        connect( d->updateSourcesInfo, SIGNAL(reset()), this, SLOT(refresh()) );
        
        connect( d->updateSourcesInfo, SIGNAL(aboutToAddUpdateSourceInfo(int,UpdateSourceInfo)),
                this, SLOT(slotAboutToAddUpdateSourceInfo(int)) );
        connect( d->updateSourcesInfo, SIGNAL(updateSourceInfoAdded(UpdateSourceInfo)),
                this, SLOT(slotUpdateSourceInfoAdded()) );
        
        connect( d->updateSourcesInfo, SIGNAL(aboutToRemoveUpdateSourceInfo(UpdateSourceInfo)),
                this, SLOT(slotAboutToRemoveUpdateSourceInfo(UpdateSourceInfo)) );
        connect( d->updateSourcesInfo, SIGNAL(updateSourceInfoRemoved(UpdateSourceInfo)),
                this, SLOT(slotUpdateSourceInfoRemoved()) );

        connect( d->updateSourcesInfo, SIGNAL(updateSourceInfoChanged(UpdateSourceInfo,UpdateSourceInfo)),
                this, SLOT(slotUpdateSourceInfoChanged(UpdateSourceInfo)) );
    }

    d->refresh();
}

/*!
   Returns a pointer to the \ref KDUpdater::UpdateSourcesInfo object whose information this
   model is containing.
*/
UpdateSourcesInfo* UpdateSourcesModel::updateSourcesInfo() const
{
    return d->updateSourcesInfo;
}

/*!
   Call this slot to reload the updates information. By default this slot is connected to
   \ref KDUpdater::UpdateSourcesInfo::reset() signal in \ref setUpdateSourcesInfo().
*/
void UpdateSourcesModel::Private::refresh()
{
    q->reset();
}

/*!
 \reimp
*/
int UpdateSourcesModel::rowCount( const QModelIndex& parent ) const
{
    if( !d->updateSourcesInfo )
        return 0;
    return parent.isValid() ? 0 : d->updateSourcesInfo->updateSourceInfoCount();
}

/*!
 \reimp
*/
int UpdateSourcesModel::columnCount( const QModelIndex& parent ) const
{
    return parent.isValid() ? 0 : 3;
}
 
/*!
 \reimp
*/
bool UpdateSourcesModel::removeRows( int row, int count, const QModelIndex& parent )
{
    if( parent.isValid() )
        return false;

    if( !d->updateSourcesInfo )
        return false;

    if( row < 0 || row + count > d->updateSourcesInfo->updateSourceInfoCount() )
        return false;

    for( int i = row; i < row + count; ++i )
        d->updateSourcesInfo->removeUpdateSourceInfoAt( i );

    return true;
}

/*!
 \reimp
*/
bool UpdateSourcesModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
    const QVariant oldData = index.data( role );
    if( oldData.isNull() )
        return false;

    if( role == Qt::UserRole )
    {
        const UpdateSourceInfo info = qVariantValue< UpdateSourceInfo >( value );
        d->updateSourcesInfo->setUpdateSourceInfoAt( index.row(), info );
        return true;
    }
    else if( role == Qt::EditRole || role == Qt::DisplayRole )
    {
        UpdateSourceInfo info = qVariantValue< UpdateSourceInfo >( oldData );
        switch( index.column() )
        {
        case Name:
            info.name = value.toString();
            break;
        case Title:
            info.title = value.toString();
            break;
        case Url:
            info.url = value.toString();
            break;
        default:
            return false;
        }
        d->updateSourcesInfo->setUpdateSourceInfoAt( index.row(), info );
        return true;
    }
    else
    {
        return false;
    }
}

/*!
 \reimp
*/
QVariant UpdateSourcesModel::data( const QModelIndex& index, int role ) const
{
    if( !index.isValid() || !d->updateSourcesInfo )
        return QVariant();

    const UpdateSourceInfo info = d->updateSourcesInfo->updateSourceInfo( index.row() );

    if( role == Qt::UserRole )
        return qVariantFromValue( info );

    if( role != Qt::DisplayRole )
        return QVariant();

    switch( index.column() )
    {
    case Name:
        return info.name;
    case Title:
        return info.title;
    case Url:
        return info.url.toString();
    default:
        return QVariant();
    }
}

/*!
 \reimp
*/
QVariant UpdateSourcesModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    if( role != Qt::DisplayRole || orientation != Qt::Horizontal )
        return QAbstractTableModel::headerData( section, orientation, role );

    switch( section )
    {
    case Name:
        return tr( "Name" );
    case Title:
        return tr( "Title" );
    case Url:
        return tr( "URL" );
    default:
        return QAbstractTableModel::headerData( section, orientation, role );
    }
}

/*!
 \internal
*/
void UpdateSourcesModel::Private::slotAboutToAddUpdateSourceInfo( int index )
{
    Q_ASSERT( updateSourcesInfo );
    q->beginInsertRows( QModelIndex(), index, index );
}

/*!
   \internal
*/
void UpdateSourcesModel::Private::slotUpdateSourceInfoAdded()
{
    Q_ASSERT( updateSourcesInfo );
    q->endInsertRows();
}

/*!
 \internal
*/
void UpdateSourcesModel::Private::slotAboutToRemoveUpdateSourceInfo( const UpdateSourceInfo& info )
{
    Q_ASSERT( updateSourcesInfo );
    for( int i = 0; i < updateSourcesInfo->updateSourceInfoCount(); ++i )
    {
        if( info == updateSourcesInfo->updateSourceInfo( i ) )
        {
            q->beginRemoveRows( QModelIndex(), i, i );
            return;
        }
    }
}

/*!
   \internal
*/
void UpdateSourcesModel::Private::slotUpdateSourceInfoRemoved()
{
    Q_ASSERT( updateSourcesInfo );
    q->endRemoveRows();
}

/*!
   \internal
*/
void UpdateSourcesModel::Private::slotUpdateSourceInfoChanged( const UpdateSourceInfo& info )
{
    Q_ASSERT( updateSourcesInfo );

    for( int i = 0; i < updateSourcesInfo->updateSourceInfoCount(); ++i )
    {
        if( info == updateSourcesInfo->updateSourceInfo( i ) )
        {
            emit q->dataChanged( q->index( i, 0 ), q->index( i, q->columnCount() -1 ) );
            return;
        }
    }
}

#include "moc_kdupdaterupdatesourcesmodel.cpp"
