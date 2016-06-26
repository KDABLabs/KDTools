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

#include "kdsemaphorereleaser.h"

#include <QSemaphore>

/*!
  \class KDSemaphoreReleaser
  \ingroup raii core
  \brief Exception-safe and convenient wrapper around QSemaphore::release()
  \since_c 2.3

  All methods in this class are nothrow.

  Semaphores differ from mutexes in that semaphore acquirers and
  releasers usually live on separate threads. So a RAII class a la
  QMutexLocker doesn't make much sense for semaphores.

  It is occasionally important to ensure, however, that the release
  operation is called reliably, even in the face of early returns or
  exceptions.

  This is where KDSemaphoreReleaser comes in. It simply stores the
  constructor arguments, thereby deferring the QSemaphore::release()
  call until the current scope's exit.

  There is no KDSemaphoreAcquirer, since the acquire operation is
  usually done before other, in particular, exception-throwing code is
  executed, so a RAII approach makes little sense for the acquire
  operation.

  Example (using QSemaphore as a multi-thread join):

  Manager Thread:
  \code
  QSemaphore sem;
  const int numWorkItems = ...;
  for ( int i = 0 ; i < numWorkItems ; ++it )
     startWorkItem( i, sem );
  sem.acquire( numWorkItems ); // join work items
  \endcode

  Work Item:
  \code
  void doWork( int i, QSemaphore & sem ) {
      const KDSemaphoreReleaser releaser( sem );
      // do work, might throw execeptions
      if ( early return )
          return;
  } // reliably calls sem.release() when exiting this scope
  \endcode
*/


/*!
  Constructor. Schedules a \a sem->release( \a count ) call at scope exit.
*/
KDSemaphoreReleaser::KDSemaphoreReleaser( QSemaphore * sem, int count )
    : sem( sem ), count( count )
{

}

/*!
  \overload
*/
KDSemaphoreReleaser::KDSemaphoreReleaser( QSemaphore & sem, int count )
    : sem( &sem ), count( count )
{

}

/*!
  Destructor. Calls QSemaphore::release() with the arguments passed
  earlier to the constructor.
*/
KDSemaphoreReleaser::~KDSemaphoreReleaser() {
    if ( sem ) sem->release( count );
}
