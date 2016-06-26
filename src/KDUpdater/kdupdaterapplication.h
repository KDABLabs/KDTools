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

#ifndef __KDTOOLS_KDUPDATERAPPLICATION_H__
#define __KDTOOLS_KDUPDATERAPPLICATION_H__

#ifndef KDTOOLS_NO_COMPAT

#include "kdupdatertarget.h"

#include <QtCore/QString>

namespace KDUpdater {

    class KDTOOLS_UPDATER_EXPORT Application : public Target 
    {
    public:
        Application();

        void setApplicationDirectory( const QString & dir ) { setDirectory( dir ); }
        QString applicationDirectory() const { return directory(); }

        QString applicationName() const { return name(); }
        QString applicationVersion() const { return version(); }
    };

}
#endif // KDTOOLS_NO_COMPAT

#endif /* __KDTOOLS_KDUPDATERAPPLICATION_H__ */
