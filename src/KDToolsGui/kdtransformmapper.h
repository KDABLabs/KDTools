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

#ifndef __KDTOOLS__GUI__KDTRANSFORMMAPPER_H__
#define __KDTOOLS__GUI__KDTRANSFORMMAPPER_H__

#include <KDToolsCore/kdtoolsglobal.h>

#include <QMatrix>
#if QT_VERSION >= 0x040300
# include <QTransform>
#endif

class KDTOOLSGUI_EXPORT KDMatrixMapper KDAB_FINAL_CLASS {
public:
    explicit KDMatrixMapper( const QMatrix & m_ ) : m( &m_ ) {};

    QPoint    operator()( const QPoint    & p ) const { return m->map( p ); }
    QPointF   operator()( const QPointF   & p ) const { return m->map( p ); }
    QLine     operator()( const QLine     & l ) const { return m->map( l ); }
    QLineF    operator()( const QLineF    & l ) const { return m->map( l ); }
    QPolygon  operator()( const QPolygon  & p ) const { return m->map( p ); }
    QPolygonF operator()( const QPolygonF & p ) const { return m->map( p ); }
    QRegion   operator()( const QRegion   & r ) const { return m->map( r ); }
    QPainterPath operator()( const QPainterPath & p ) const { return m->map( p ); }
    QRectF    operator()( const QRectF    & r ) const { return m->mapRect( r ); }
    QRect     operator()( const QRect     & r ) const { return m->mapRect( r ); }

private:
    const QMatrix * const m;
};

#if defined(DOXYGEN_RUN) || QT_VERSION >= 0x040300
class KDTOOLSGUI_EXPORT KDTransformMapper KDAB_FINAL_CLASS {
public:
    explicit KDTransformMapper( const QTransform & t_ ) : t( &t_ ) {};

    QPoint    operator()( const QPoint    & p ) const { return t->map( p ); }
    QPointF   operator()( const QPointF   & p ) const { return t->map( p ); }
    QLine     operator()( const QLine     & l ) const { return t->map( l ); }
    QLineF    operator()( const QLineF    & l ) const { return t->map( l ); }
    QPolygon  operator()( const QPolygon  & p ) const { return t->map( p ); }
    QPolygonF operator()( const QPolygonF & p ) const { return t->map( p ); }
    QRegion   operator()( const QRegion   & r ) const { return t->map( r ); }
    QPainterPath operator()( const QPainterPath & p ) const { return t->map( p ); }
    QRectF    operator()( const QRectF    & r ) const { return t->mapRect( r ); }
    QRect     operator()( const QRect     & r ) const { return t->mapRect( r ); }

private:
    const QTransform * const t;
};
#endif // defined(DOXYGEN_RUN) || QT_VERSION >= 0x040300

#endif /* __KDTOOLS__GUI__KDTRANSFORMMAPPER_H__ */

