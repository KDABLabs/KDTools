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

#include "kdupdaterupdatesmodel.h"

#include "kdupdaterupdate.h"
#include "kdupdaterpackagesinfo.h"

using namespace KDUpdater;

/*!
   \ingroup kdupdater
   \class KDUpdater::UpdatesModel kdupdaterupdatesmodel.h KDUpdaterUpdatesModel
   \brief An item model usable for item views that let the user chooses which updates he wants to install
   \since_c 2.3

   After \ref KDUpdater::UpdateFinder class finds all updates available for an target,
   this item model can be used to help the user select which update he wants to install.

   For each update the model contains a row with the following data:

   \li %Selection state as Qt::CheckState in Qt::CheckStateRole,
   \li %Update name,
   \li New version and
   \li Human readable compressed size of the %Update, if any as QString in Qt::DisplayRole

   Additionally, the %Update itself is available as Update* in Qt::UserRole.

   Alternatively, you can use the prefedined \ref KDUpdater::UpdatesDialog.

   Usage:
   \code
    QList<KDUpdater::Update*> updates = updateFinder.updates();

    KDUpdater::UpdatesModel model(this);
    model.setUpdates(updates);

    // some custom dialog
    updatesDialog.setModel( &model );
    if( updatesDialog.exec() != QDialog::Accepted )
    {
        qDeleteAll(updates);
        updates.clear();
        return;
    }

    // these are the updates to install
    QList<KDUpdater::Update*> reqUpdates = model.selectedUpdates();
    // ...    
    \endcode
*/

/*!
 \internal
 */
class UpdatesModel::Private
{
public:
    explicit Private( UpdatesModel* qq )
        : q( qq )
    {
    }

private:
    UpdatesModel* const q;

public:
    /*!
     Sets the list of available updates.
     \internal
     */
    void setUpdates( const QList< Update* >& newUpdates )
    {
        selectedUpdates.clear();
        updates = newUpdates;
        if( updates.count() == 1 )
            selectedUpdates.push_back( updates.front() );
    }

    /*!
     Returns the size \a s as nice looking file size with a proper byte unit.
     \internal
     */
    static QString niceSizeText( quint64 s )
    {
        if( s == 0 )
            return QString();
        double size = s;
        if( size < 10000.0 )
            return tr( "%L1 Bytes" ).arg( size );
        size /= 1024.0;
        if( size < 10000.0 )
            return tr( "%L1 kB" ).arg( size, 0, 'f', 1 );
        size /= 1024.0;
        if( size < 10000.0 )
            return tr( "%L1 MB" ).arg( size, 0, 'f', 1 );
        size /= 1024.0;
        return tr( "%L1 GB" ).arg( size, 0, 'f', 1 );
    }

    QList< Update* > updates;
    QList< Update* > selectedUpdates;
};

/*!
 Creates a new UpdatesModel with \a parent.
 */
UpdatesModel::UpdatesModel( QObject* parent )
    : QAbstractTableModel( parent ),
      d( new Private( this ) )
{
}

/*!
 Destroys the UpdatesModel.
 */
UpdatesModel::~UpdatesModel()
{
}

/*!
 Set the \a updates contained in this model. This will reset the selection state.
 */
void UpdatesModel::setUpdates( const QList< Update* >& updates )
{
#if QT_VERSION >= 0x040600
    beginResetModel();
    d->setUpdates( updates );
    endResetModel();
#else
    d->setUpdates( updates );
    reset();
#endif
}

/*!
 Returns the updates contained by this model.
 */
QList< Update* > UpdatesModel::updates() const
{
    return d->updates;
}
 
/*!
 Returns the list of updates selected within this model (i.e. Qt::CheckStateRole in column 0 returns Qt::Checked)
 */
QList< Update* > UpdatesModel::selectedUpdates() const
{
    return d->selectedUpdates;
}

/*!
 \reimp
 */
int UpdatesModel::rowCount( const QModelIndex& parent ) const
{
    return parent.isValid() ? 0 : d->updates.count();
}

/*!
 \reimp
 */
int UpdatesModel::columnCount( const QModelIndex& parent ) const
{
    // selected
    // name
    // version
    // size
    return parent.isValid() ? 0 : 4;
}

/*!
 \reimp
 */
Qt::ItemFlags UpdatesModel::flags( const QModelIndex& index ) const
{
    Qt::ItemFlags f = QAbstractTableModel::flags( index );
    if( index.column() == 0 )
        f |= Qt::ItemIsUserCheckable;
    return f;
}

/*!
 \reimp
 */
QVariant UpdatesModel::data( const QModelIndex& index, int role ) const
{
    if( !index.isValid() || index.row() >= d->updates.count() )
        return QVariant();
    
    Update* const update = d->updates.at( index.row() );
    switch( role )
    {
    case Qt::UserRole:
        return qVariantFromValue( update );
    case Qt::DisplayRole:
    case Qt::EditRole:
        switch( index.column() )
        {
        case 1: // name
            return update->data( QLatin1String( "Name" ) );
        case 2: // version
            return update->data( QLatin1String( "Version" ) );
        case 3: // size
            return Private::niceSizeText( update->compressedSize() );
        }
        break;
    case Qt::CheckStateRole:
        switch( index.column() )
        {
        case 0: // selected?
            return d->selectedUpdates.contains( update ) ? Qt::Checked : Qt::Unchecked;
        }
        break;
    }
    return QVariant();
}
        
/*!
 \reimp
 */
QVariant UpdatesModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    if( orientation == Qt::Horizontal && role == Qt::DisplayRole )
    {
        switch( section )
        {
        case 0:
            return tr( "Install" );
        case 1:
            return tr( "Name" );
        case 2:
            return tr( "Version" );
        case 3:
            return tr( "Size" );
        }
    }
    return QAbstractTableModel::headerData( section, orientation, role );
}

/*!
 \reimp
 */
bool UpdatesModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
    if( !index.isValid() || index.row() >= d->updates.count() )
        return false;
    
    Update* const update = d->updates.at( index.row() );
    switch( role )
    {
    case Qt::CheckStateRole:
        switch( value.toInt() )
        {
        case Qt::Checked:
            if( !d->selectedUpdates.contains( update ) )
                d->selectedUpdates.push_back( update );
            emit dataChanged( index, index );
            return true;
        case Qt::Unchecked:
            d->selectedUpdates.removeAll( update );
            emit dataChanged( index, index );
            return true;
        }
    }
    return false;
}
