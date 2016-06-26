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

#ifndef __KDTOOLS__CORE__KDSHAREDMEMORYLOCKER_H
#define __KDTOOLS__CORE__KDSHAREDMEMORYLOCKER_H

#include <KDToolsCore/kdtoolsglobal.h>

#if QT_VERSION >= 0x040400 || defined( DOXYGEN_RUN )
#ifndef QT_NO_SHAREDMEMORY

QT_BEGIN_NAMESPACE
class QSharedMemory;
QT_END_NAMESPACE

#ifndef DOXYGEN_RUN
namespace kdtools {
#endif

class KDTOOLSCORE_EXPORT KDSharedMemoryLocker KDAB_FINAL_CLASS {
    Q_DISABLE_COPY( KDSharedMemoryLocker )
public:
    explicit KDSharedMemoryLocker( QSharedMemory * mem );
    explicit KDSharedMemoryLocker( QSharedMemory & mem );
    ~KDSharedMemoryLocker();

private:
    QSharedMemory * const mem;
};

#ifndef DOXYGEN_RUN
}
#endif

#endif /* QT_NO_SHAREDMEMORY */

#endif /* QT_VERSION >= 0x040400 || defined( DOXYGEN_RUN ) */

#endif /* __KDTOOLS__CORE__KDSHAREDMEMORYLOCKER_H */
