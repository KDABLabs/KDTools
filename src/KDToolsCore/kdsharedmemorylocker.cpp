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

#include "kdsharedmemorylocker.h"

#if QT_VERSION >= 0x040400 || defined( DOXYGEN_RUN )

#ifndef QT_NO_SHAREDMEMORY

#include <QSharedMemory>

using namespace kdtools;

/*!
  \class KDSharedMemoryLocker
  \ingroup raii core
  \brief Exception-safe and convenient wrapper around QSharedMemory::lock()
*/

/*!
  Constructor. Locks the shared memory segment \a m.
  If another process has locking the segment, this constructor blocks
  until the lock is released. The memory segments needs to be properly created or attached.
*/
KDSharedMemoryLocker::KDSharedMemoryLocker( QSharedMemory * m )
    : mem( m )
{
    if ( mem )
        mem->lock();
}

/*!
  Constructor.
  \overload
*/
KDSharedMemoryLocker::KDSharedMemoryLocker( QSharedMemory & m )
    : mem( &m )
{
    if ( mem )
        mem->lock();
}

/*!
  Destructor. Unlocks the shared memory segment associated with this
  KDSharedMemoryLocker.
*/
KDSharedMemoryLocker::~KDSharedMemoryLocker() {
    if ( mem )
        mem->unlock();
}

#endif /* QT_NO_SHAREDMEMORY */

#endif /* QT_VERSION >= 0x040400 || defined( DOXYGEN_RUN ) */
