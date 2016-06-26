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

#ifndef __KDTOOLS_KDUPDATERUPDATESMODEL_H__
#define __KDTOOLS_KDUPDATERUPDATESMODEL_H__

#include "kdupdater.h"
#include <KDToolsCore/pimpl_ptr.h>

#include <QtCore/QAbstractTableModel>

namespace KDUpdater
{
    class Update;

    class KDTOOLS_UPDATER_EXPORT UpdatesModel : public QAbstractTableModel
    {
        Q_OBJECT
    public:
        explicit UpdatesModel( QObject* parent = 0 );
        ~UpdatesModel();

        void setUpdates( const QList< Update* >& updates );
        QList< Update* > updates() const;

        QList< Update* > selectedUpdates() const;

        Qt::ItemFlags flags( const QModelIndex& index ) const;
        QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
        QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
        bool setData( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole );

        int columnCount( const QModelIndex& parent = QModelIndex() ) const;
        int rowCount( const QModelIndex& parent = QModelIndex() ) const;

    private:
        class Private;
        kdtools::pimpl_ptr< Private > d;
    };
}

Q_DECLARE_METATYPE( KDUpdater::Update* )

#endif
