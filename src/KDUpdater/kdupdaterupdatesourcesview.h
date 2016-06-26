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

#ifndef __KDTOOLS_KDUPDATERUPDATESOURCESVIEW_H__
#define __KDTOOLS_KDUPDATERUPDATESOURCESVIEW_H__

#include "kdupdater.h"
#include <KDToolsCore/pimpl_ptr.h>

#include <QtGui/QDialog>
#include <QtGui/QTreeView>

namespace KDUpdater
{
    struct UpdateSourceInfo;
    class UpdateSourcesInfo;

    class KDTOOLS_UPDATER_EXPORT EditUpdateSourceDialog : public QDialog
    {
        Q_OBJECT
    public:
        explicit EditUpdateSourceDialog( QWidget* parent = 0 );
        ~EditUpdateSourceDialog();

        void setInfo( const UpdateSourceInfo& info );
        UpdateSourceInfo info() const;

    public Q_SLOTS:
        void accept();

    private:
        class Private;
        kdtools::pimpl_ptr< Private > d;
    };

    class KDTOOLS_UPDATER_EXPORT UpdateSourcesView : public QTreeView
    {
        Q_OBJECT

    public:
        explicit UpdateSourcesView( QWidget* parent = 0 );
        ~UpdateSourcesView();

        void setUpdateSourcesInfo(UpdateSourcesInfo* info);
        UpdateSourcesInfo* updateSourcesInfo() const;

        int currentUpdateSourceInfoIndex() const;

    public Q_SLOTS:
        void addNewSource();
        void removeCurrentSource();
        void editCurrentSource();

    protected:
#ifndef QT_NO_CONTEXTMENU
        void contextMenuEvent( QContextMenuEvent* e );
#endif

    private:
        class Private;
        kdtools::pimpl_ptr< Private > d;
    };

}

#endif
