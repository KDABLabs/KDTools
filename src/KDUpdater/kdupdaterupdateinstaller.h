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

#ifndef __KDTOOLS_KDUPDATERUPDATEINSTALLER_H__
#define __KDTOOLS_KDUPDATERUPDATEINSTALLER_H__

#include "kdupdater.h"
#include "kdupdatertask.h"
#include <KDToolsCore/pimpl_ptr.h>

#ifndef KDTOOLS_NO_COMPAT
# include "kdupdaterapplication.h"
#endif // KDTOOLS_NO_COMPAT

QT_BEGIN_NAMESPACE
template< typename T >
class QList;
QT_END_NAMESPACE

namespace KDUpdater
{
    class Target;
    class Update;

    class KDTOOLS_UPDATER_EXPORT UpdateInstaller : public Task
    {
        Q_OBJECT

    public:
        explicit UpdateInstaller( Target * target );
        ~UpdateInstaller();

        Target * target() const;

        void setUpdatesToInstall(const QList<Update*>& updates);
        QList<Update*> updatesToInstall() const;

#ifndef KDTOOLS_NO_COMPAT
        Application * application() const { return dynamic_cast<Application*>( target() ); }
#endif // KDTOOLS_NO_COMPAT

    private:
        void doRun();
        bool doStop();
        bool doPause();
        bool doResume();

        bool installUpdate(Update* update, int minPc, int maxPc);

        class Private;
        kdtools::pimpl_ptr<Private> d;

        Q_PRIVATE_SLOT( d, void slotUpdateDownloadProgress() )
        Q_PRIVATE_SLOT( d, void slotUpdateDownloadDone() )
        Q_PRIVATE_SLOT( d, void slotUpdateDownloadFailed() )
    };

}

#endif
