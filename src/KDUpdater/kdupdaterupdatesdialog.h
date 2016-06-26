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

#ifndef __KDTOOLS_KDUPDATERUPDATESDIALOG_H__
#define __KDTOOLS_KDUPDATERUPDATESDIALOG_H__

#include "kdupdater.h"
#include <KDToolsCore/pimpl_ptr.h>

#include <QtGui/QDialog>

QT_BEGIN_NAMESPACE
template< typename T >
class QList;
QT_END_NAMESPACE

namespace KDUpdater
{
    class Update;

    class KDTOOLS_UPDATER_EXPORT UpdatesDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit UpdatesDialog(QWidget *parent = 0);
        ~UpdatesDialog();

        void setUpdates(const QList<Update*> &updates);
        QList<Update*> updates() const;

        bool isUpdateAllowed( const Update * update ) const;

    private:
        Q_PRIVATE_SLOT( d, void slotStateChanged() )
        Q_PRIVATE_SLOT( d, void slotPreviousClicked() )
        Q_PRIVATE_SLOT( d, void slotNextClicked() )

        class Private;
        kdtools::pimpl_ptr< Private > d;
    };
}

#endif
