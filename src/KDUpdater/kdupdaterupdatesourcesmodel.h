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

#ifndef __KDTOOLS_KDUPDATERUPDATESOURCESMODEL_H__
#define __KDTOOLS_KDUPDATERUPDATESOURCESMODEL_H__

#include "kdupdater.h"
#include <KDToolsCore/pimpl_ptr.h>

#include <QtCore/QAbstractTableModel>

namespace KDUpdater
{
    class UpdateSourcesInfo;

    class KDTOOLS_UPDATER_EXPORT UpdateSourcesModel : public QAbstractTableModel
    {
        Q_OBJECT

    public:
        enum Column
        {
            Name,
            Title,
            Url
        };

        explicit UpdateSourcesModel( QObject* parent = 0 );
        ~UpdateSourcesModel();

        void setUpdateSourcesInfo( UpdateSourcesInfo* info );
        UpdateSourcesInfo* updateSourcesInfo() const;

        int rowCount( const QModelIndex& parent = QModelIndex() ) const;
        int columnCount( const QModelIndex& parent = QModelIndex() ) const;

        bool removeRows( int row, int count, const QModelIndex& parent = QModelIndex() );

        bool setData( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole );
        QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
        QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

    private:
        Q_PRIVATE_SLOT( d, void refresh() )
        
        Q_PRIVATE_SLOT( d, void slotAboutToAddUpdateSourceInfo( int index ) )
        Q_PRIVATE_SLOT( d, void slotUpdateSourceInfoAdded() )
        
        Q_PRIVATE_SLOT( d, void slotAboutToRemoveUpdateSourceInfo( const UpdateSourceInfo& info ) )
        Q_PRIVATE_SLOT( d, void slotUpdateSourceInfoRemoved() )
        
        Q_PRIVATE_SLOT( d, void slotUpdateSourceInfoChanged( const UpdateSourceInfo& newInfo ) )

        class Private;
        kdtools::pimpl_ptr< Private > d;
    };

}

#endif
