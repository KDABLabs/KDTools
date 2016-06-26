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

#include "kdpaintersaver.h"

#include <QPainter>

/*!
  \class KDPainterSaver
  \ingroup raii gui
  \brief Exception-safe and convenient wrapper around QPainter::save()/\link QPainter::restore() restore()\endlink

  \note KDTools 2.3 added more specialized versions of this class:
  KDPenSaver, KDBrushSaver, KDFontSaver, and KDTransformSaver

  This class automates the task of matching QPainter::save() with
  QPainter::restore() calls. If you always use this class instead of
  direct calls, you can never forget to call \link QPainter::restore() restore()\endlink.

  This is esp. important when dealing with code that might throw
  exceptions, or functions with many return statements.

  \code
  void MyClass::drawRotatedLabel( QPainter * p, const QPoint & pos, double deg ) {
      p->save();
      p->translate( pos.x(), pos.y() );
      p->rotate( deg );
      const QString l = label();
      if ( l.isEmpty() ) {
          p->restore();
          return;
      }
      p->drawText( 0, 0, l );
      p->restore();
  }
  \endcode

  In this case, if \c label() throws, the painter will not be properly
  restored.

  \code
  void MyClass::drawRotatedLabel( QPainter * p, const QPoint & pos, double deg ) {
      const KDPainterSaver ps( p );
      p->translate( pos.x(), pos.y() );
      p->rotate( deg );
      const QString l = label();
      if ( l.isEmpty() )
          return;
      p->drawText( 0, 0, l );
  }
  \endcode

  Using KDPainterSaver, \c p is always properly restored, even if \c
  label() throws an exception, or we take an early return.

  All functions in this class are nothrow if QPainter::save() and
  QPainter::restore() are nothrow, which they normally are.

  \sa KDPenSaver, KDBrushSaver, KDFontSaver, KDTransformSaver
*/

/*!
  Constructor. Calls \link QPainter::save() save()\endlink on \a p.
*/
KDPainterSaver::KDPainterSaver( QPainter * p )
    : painter( p )
{
    if ( painter )
        painter->save();
}

/*!
  \since_f 2.3
  \overload
*/
KDPainterSaver::KDPainterSaver( QPainter & p )
    : painter( &p )
{
    p.save();
}

/*!
  Destructor. Calls \link QPainter::restore() restore()\endlink on the contained painter.
*/
KDPainterSaver::~KDPainterSaver() {
    if ( painter )
        painter->restore();
}

#define IMPL_PAINTER_SAVER( Type, type, KDT_Version )       \
    /*!
      \class KD##Type##Saver
      \ingroup raii gui
      \since_c KDT_Version
      \brief Exception-safe and convenient wrapper around QPainter::set##Type##()

      <p>
      This is a more specialized version of KDPainterSaver. Instead of
      saving all of the painter's state, KD##Type##Saver only saves
      and restores the painter's type. This can be much faster than
      using the full KDPainterSaver.

    */                                          \
                                                \
    /*!
      Constructor. Sets \a p's type to \a type after saving the old value.
    */                                                                  \
    KD##Type##Saver::KD##Type##Saver( QPainter * p, const Q##Type & type ) \
        : type( p ? p->type() : Q##Type() ), painter( p )               \
    {                                                                   \
        if ( p ) p->set##Type( type );                                  \
    }                                                                   \
                                                                        \
    /*!
      \overload
    */                                                                  \
    KD##Type##Saver::KD##Type##Saver( QPainter & p, const Q##Type & type ) \
        : type( p.type() ), painter( &p )                               \
    {                                                                   \
        p.set##Type( type );                                            \
    }                                                                   \
                                                                        \
    /*!
      Constructor. Saves, but does not alter \a p's type.
    */                                                                  \
    KD##Type##Saver::KD##Type##Saver( QPainter * p )                    \
        : type( p ? p->type() : Q##Type() ), painter( p ) {}            \
                                                                        \
    /*!
      \overload
    */                                                                  \
    KD##Type##Saver::KD##Type##Saver( QPainter & p )                    \
        : type( p.type() ), painter( &p ) {}                            \
                                                                        \
    /*!
      Destructor. Restores the painter's type to its original value.
    */                                                                  \
    KD##Type##Saver::~KD##Type##Saver()                                 \
    {                                                                   \
        if ( painter ) painter->set##Type( type );                      \
    }

IMPL_PAINTER_SAVER( Pen, pen, 2.3 )
IMPL_PAINTER_SAVER( Brush, brush, 2.3 )
IMPL_PAINTER_SAVER( Font, font, 2.3 )
IMPL_PAINTER_SAVER( Transform, transform, 2.3 )

#undef IMPL_PAINTER_SAVER
