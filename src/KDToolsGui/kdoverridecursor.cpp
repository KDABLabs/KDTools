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

#include "kdoverridecursor.h"

#ifndef QT_NO_CURSOR

#include <QApplication>

/*!
  \class KDOverrideCursor
  \ingroup raii gui
  \brief Exception-safe and convenient wrapper around QApplication::setOverrideCursor()

  This class automates the task of matching
  QApplication::setOverrideCursor() calls with
  QApplication::restoreOverrideCursor() calls. If you always use this
  class instead of direct calls, you can never forget to call
  \link QApplication::restoreOverrideCursor() restoreOverrideCursor()\endlink.

  This is esp. important when dealing with code that might throw
  exceptions.

  Sometimes, it is necessary to temporarily disable the override
  cursor, eg. when interrupting a long operation to show a message
  box. You can use disable() and enable() to achieve this:

  \code
  KDOverrideCursor wait;
  // perform long-running operation...
  wait.disable();
  QMessageBox::information( this, tr("Information"), tr("This will not show an hourglass cursor") );
  wait.enable();
  // resume long-running operation...
  \endcode

  As with all pairing of function calls, this construct is not
  exception-safe. This is normally not a problem, since the call to
  \link QApplication::restoreOverrideCursor()
  restoreOverrideCursor()\endlink in the destructor is inhibited when
  \c wait is disabled, so set/restoreOverrideCursor calls are still nicely
  matched, but if the code inbetween the disable() and enable() calls
  can throw, and you nest disable()/\link enable() enable()\endlink,
  use of KDDisableOverrideCursor is strongly advised. Deeper nesting
  can be achieved with creating a new KDOverrideCursor instance in the
  inner block, and so on.

  All functions in this class are nothrow, if
  QApplication::setOverrideCursor() and
  QApplication::restoreOverrideCursor() are nothrow, which they
  normally are.
*/

/*!
  Constructs a KDOverrideCursor with Qt::WaitCursor as cursor. In the
  vast majority of cases, this is what you need.

  \post QApplication::overrideCursor() == Qt::WaitCursor
*/
KDOverrideCursor::KDOverrideCursor()
    : cursor( Qt::WaitCursor ),
      enabled( true )
{
    QApplication::setOverrideCursor( cursor );
}

/*!
  Constructs a KDOverrideCursor with \a c as cursor.

  \post QApplication::overrideCursor() == \a c
*/
KDOverrideCursor::KDOverrideCursor( const QCursor & c )
    : cursor( c ),
      enabled( true )
{
    QApplication::setOverrideCursor( cursor );
}

/*!
  Restores the previous override cursor.

  \post QApplication::overrideCursor() == cursor before the object was constructed
*/
KDOverrideCursor::~KDOverrideCursor() {
    if ( enabled )
        QApplication::restoreOverrideCursor();
}

/*!
  Temporarily disables the override cursor.

  \post QApplication::overrideCursor() == cursor before this object was constructed

  \sa enable()
*/
void KDOverrideCursor::disable() {
    if ( !enabled )
        return;
    QApplication::restoreOverrideCursor();
    enabled = false;
}

/*!
  Re-enables the override cursor.

  \post QApplication::overrideCursor() == this->cursor

  \sa disable()
*/
void KDOverrideCursor::enable() {
    if ( enabled )
        return;
    QApplication::setOverrideCursor( cursor );
    enabled = true;
}


/*!
  \class KDDisableOverrideCursor
  \ingroup raii gui
  \brief Exception-safe and convenient way to temporarily disable a KDOverrideCursor

  Constructing a KDDisableOverrideCursor over a KDOverrideCursor
  temporarily disables the latter for the lifetime of the
  KDDisableOverrideCursor instance. See KDOverrideCursor for more on
  this.

  All methods in this class are nothrow, if
  QApplication::setOverrideCursor() and
  QApplication::restoreOverrideCursor() are nothrow, which they
  usually are.
*/

/*!
  \fn KDDisableOverrideCursor::KDDisableOverrideCursor( KDOverrideCursor & orc )

  Disables \a orc.
*/
KDDisableOverrideCursor::KDDisableOverrideCursor( KDOverrideCursor & orc )
    : overrideCursor( orc )//, enabled( orc.enabled )
{
    overrideCursor.disable();
}

/*!
  \fn KDDisableOverrideCursor::~KDDisableOverrideCursor()

  Re-enables the KDOverrideCursor we were constructed on.
*/
KDDisableOverrideCursor::~KDDisableOverrideCursor() {
    overrideCursor.enable();
}

#endif // QT_NO_CURSOR
