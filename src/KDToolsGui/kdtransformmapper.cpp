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

#include "kdtransformmapper.h"

/*!
  \class KDMatrixMapper
  \ingroup gui
  \brief A class to turn a QMatrix into a functor
  \since_c 2.3

  This class is identical to KDTransformMapper, except that it
  operates on QMatrix instead of QTransform. For a detailed
  description, please see KDTransformMapper.
*/

/*!
  \fn KDMatrixMapper::KDMatrixMapper( const QMatrix & m )
  Constructs a KDMatrixMapper that wraps \a m.
  Since KDMatrixMapper internally only stores a pointer to \a m,
  the lifetime of \a m needs to exceed that of the KDMatrixMapper
  instance.
*/

/*!
  \fn QPoint    KDMatrixMapper::operator()( const QPoint    & p ) const
  Same as KDTransformMapper::operator()( const QPoint    & p ) const.
*/

/*!
  \fn QPointF   KDMatrixMapper::operator()( const QPointF   & p ) const
  Same as KDTransformMapper::operator()( const QPointF   & p ) const.
*/

/*!
  \fn QLine     KDMatrixMapper::operator()( const QLine     & l ) const
  Same as KDTransformMapper::operator()( const QLine     & l ) const.
*/

/*!
  \fn QLineF    KDMatrixMapper::operator()( const QLineF    & l ) const
  Same as KDTransformMapper::operator()( const QLineF    & l ) const.
*/

/*!
  \fn QPolygon  KDMatrixMapper::operator()( const QPolygon  & p ) const
  Same as KDTransformMapper::operator()( const QPolygon  & p ) const.
*/

/*!
  \fn QPolygonF KDMatrixMapper::operator()( const QPolygonF & p ) const
  Same as KDTransformMapper::operator()( const QPolygonF & p ) const.
*/

/*!
  \fn QRegion   KDMatrixMapper::operator()( const QRegion   & r ) const
  Same as KDTransformMapper::operator()( const QRegion   & r ) const.
*/

/*!
  \fn QPainterPath KDMatrixMapper::operator()( const QPainterPath & p ) const
  Same as KDTransformMapper::operator()( const QPainterPath & p ) const.
*/

/*!
  \fn QRectF    KDMatrixMapper::operator()( const QRectF    & r ) const
  Same as KDTransformMapper::operator()( const QRectF    & r ) const.
*/

/*!
  \fn QRect     KDMatrixMapper::operator()( const QRect     & r ) const
  Same as KDTransformMapper::operator()( const QRect     & r ) const.
*/



/*!
  \class KDTransformMapper
  \ingroup gui
  \brief A class to turn a QTransform into a functor
  \since_c 2.3

  This class wraps QTransform to provide a function-call operator. It
  can therefore adapt a QTransform instance such that it can be used
  as an unary function object.

  \section usage Usage

  As an example, take the following code:
  \code
  QTransform t = ...;
  QVector<QPoint> points = ...;
  for ( QVector<QPoint>::iterator it = points.begin(), end = points.end()
        it != end ; ++it )
    *it = t.map( it );
  \endcode

  We can't use Q_FOREACH here, because we need (mutable) iterators
  whereas Q_FOREACH works with const iterators internally.

  You might be tempted to write
  \code
  std::transform( points.begin(), points.end(),
                  t );
  \endcode
  or
  \code
  std::transform( points.begin(), points.end(),
                  bind( &QTransform::map, t, _1 ) );
  \endcode

  But neither work: The former doesn't compile because QTransform is
  not a function object, the latter doesn't compile because
  QTransform::map() is overloaded, and you can't take the address of
  an overloaded function without disambiguation.

  KDTransformMapper now adapts QTransform such that it's usable in the
  first transform() call we've seen:
  \code
  std::transform( points.begin(), points.end(),
                  KDTransformMapper( t ) );
  \endcode

  This works because KDTransformMapper implements the function-call
  operator that is missing from QTransform itself.

  \note Due to performance reasons, the QTransform is not copied. It
  is therefore imperative that the QTransform instance's life time
  exceeds that of the KDTransformMapper wrapping it. In most
  use-cases, this is a non-issue. Care must be taken, however, when
  named instances of KDTransformMapper are created.

  \section rects Rectangles

  QRect and QRectF are the only geometrical primitives that cannot be
  mapped into the same concept, in general. If you rotate a rectangle
  by 45 degrees, the result is no longer a rectangle that can be
  represented by the QRect/QRectF.

  For this reason, QTransform provides two mapping functions for rectangles:
  \li QTransform::mapRect(const QRect&), mapping rectangles into the \em bounding rectangles
  \li QTransform::mapToPolygon(const QRect&), mapping rectangles to rectangles represented as QPolygon's

  KDTransformMapper cannot provide both options, since it is designed
  to be used as a function object, not a traditional class, and so we
  had to pick one of the two for implementation in KDTransformMapper.

  We chose QTransform::mapRect(), for two reasons:
  \li It's more regular (return value is the same as the argument type)
  \li You can always use the following to get the polygon behaviour:
  \code
  std::transform( rects.begin(), rects.end(),
                  bind( &QTransform::mapToPolygon, t, _1 ) );
  \endcode
  because QTransform::mapToPolygon() isn't (yet) overloaded, whereas
  QTransform::mapRect() is.

  \section adapt Adaptability

  KDTransformMapper is not Adaptable in the STL sense, since it does
  not (cannot) provide a nested \c result_type typedef (the function
  call operator is overloaded, so the result type differs, depending
  on which overload is called).

  However, modern tools like bind() do not really need classic STL
  Adaptable function objects anymore, because they usually don't care
  about the argument types and provide a way to override the \c
  result_type at the point of use. For bind(), the following can be
  used:
  \code
  bind<QPoint>( KDTransformMapper( t ), _1 )
  \endcode
*/

/*!
  \fn KDTransformMapper::KDTransformMapper( const QTransform & t )
  Constructs a KDTransformMapper that wraps \a t.
  Since KDTransformMapper internally only stores a pointer to \a t,
  the lifetime of \a t needs to exceed that of the KDTransformMapper
  instance.
*/

/*!
  \fn QPoint    KDTransformMapper::operator()( const QPoint    & p ) const
  Wraps QTransform::map(const QPoint &).
*/

/*!
  \fn QPointF   KDTransformMapper::operator()( const QPointF   & p ) const
  Wraps QTransform::map(const QPointF   &).
*/

/*!
  \fn QLine     KDTransformMapper::operator()( const QLine     & l ) const
  Wraps QTransform::map(const QLine     &).
*/

/*!
  \fn QLineF    KDTransformMapper::operator()( const QLineF    & l ) const
  Wraps QTransform::map(const QLineF    &).
*/

/*!
  \fn QPolygon  KDTransformMapper::operator()( const QPolygon  & p ) const
  Wraps QTransform::map(const QPolygon  &).
*/

/*!
  \fn QPolygonF KDTransformMapper::operator()( const QPolygonF & p ) const
  Wraps QTransform::map(const QPolygonF &).
*/

/*!
  \fn QRegion   KDTransformMapper::operator()( const QRegion   & r ) const
  Wraps QTransform::map(const QRegion   &).
*/

/*!
  \fn QPainterPath KDTransformMapper::operator()( const QPainterPath & p ) const
  Wraps QTransform::map(const QPainterPath &).
*/

/*!
  \fn QRectF    KDTransformMapper::operator()( const QRectF    & r ) const
  Wraps QTransform::mapRect(const QRectF    &).
  See \ref rects for more information.
*/

/*!
  \fn QRect     KDTransformMapper::operator()( const QRect     & r ) const
  Wraps QTransform::mapRect(const QRect     &).
  See \ref rects for more information.
*/

#ifdef KDTOOLSGUI_UNITTESTS

#include <KDUnitTest/Test>

static std::ostream & operator<<( std::ostream & stream, const QPoint & point ) {
    return stream << "QPoint( " << point.x() << ", " << point.y() << " )";
}

static std::ostream & operator<<( std::ostream & stream, const QLine & line ) {
    return stream << "QLine( " << line.p1() << ", " << line.p2() << " )";
}

static std::ostream & operator<<( std::ostream & stream, const QPointF & point ) {
    return stream << "QPointF( " << point.x() << ", " << point.y() << " )";
}

static std::ostream & operator<<( std::ostream & stream, const QLineF & line ) {
    return stream << "QLineF( " << line.p1() << ", " << line.p2() << " )";
}

static const QMatrix matrices[] = {
    QMatrix(),
    QMatrix( -1, 0, 0, -1, 0, 0 ),
    QMatrix(  1, 0, 0,  1, 1, 1 ),
    QMatrix().rotate( 45 ),
    QMatrix().rotate( -45 ),
    QMatrix().rotate( 90 ),
    QMatrix().rotate( -90 ),
    QMatrix().scale( 2, 2 ),
    QMatrix().scale( 0.5, 0.5 ),
};

static const QPoint points[] = {
    QPoint( 0, 0 ),
    QPoint( 1, 0 ),
    QPoint( 0, 1 ),
};

static const QPointF pointfs[] = {
    QPointF( 0, 0 ),
    QPointF( 1, 0 ),
    QPointF( 0, 1 ),
    QPointF( 0.5, 0.5 ),
    QPointF( 3.14, 1 ),
};

static const QLine lines[] = {
    QLine( 0, 0, 0, 1 ),
    QLine( 0, 0, 1, 0 ),
    QLine( 0, 0, 1, 1 ),
    QLine( 0, 1, 0, 0 ),
    QLine( 0, 1, 1, 0 ),
    QLine( 0, 1, 1, 1 ),
    QLine( 1, 0, 0, 0 ),
    QLine( 1, 0, 0, 1 ),
    QLine( 1, 0, 1, 1 ),
    QLine( 1, 1, 0, 0 ),
    QLine( 1, 1, 0, 1 ),
    QLine( 1, 1, 1, 0 ),
};

static const QLineF linefs[] = {
    QLineF( 0, 0, 0, 1 ),
    QLineF( 0, 0, 1, 0 ),
    QLineF( 0, 0, 1, 1 ),
    QLineF( 0, 1, 0, 0 ),
    QLineF( 0, 1, 1, 0 ),
    QLineF( 0, 1, 1, 1 ),
    QLineF( 1, 0, 0, 0 ),
    QLineF( 1, 0, 0, 1 ),
    QLineF( 1, 0, 1, 1 ),
    QLineF( 1, 1, 0, 0 ),
    QLineF( 1, 1, 0, 1 ),
    QLineF( 1, 1, 1, 0 ),
};

KDAB_UNITTEST_SIMPLE( KDMatrixMapper, "kdtools/gui" ) {

#define DO( Type, samples ) \
    for ( unsigned int i = 0 ; i < sizeof samples / sizeof *samples ; ++i ) \
        for ( unsigned int j = 0 ; j < sizeof matrices / sizeof *matrices ; ++j ) \
            assertEqual( matrices[j].map( samples[i] ), KDMatrixMapper( matrices[j] )( samples[i] ) )

    DO( QPoint,  points  );
    DO( QPointF, pointfs );
    DO( QLine,   lines   );
    DO( QLineF,  linefs  );

#undef DO

}

#if QT_VERSION >= 0x040300

static const QTransform transforms[] = {
    QTransform(),
    QTransform( -1, 0, 0, -1, 0, 0 ),
    QTransform(  1, 0, 0,  1, 1, 1 ),
    QTransform().rotate( 45 ),
    QTransform().rotate( -45 ),
    QTransform().rotate( 90 ),
    QTransform().rotate( -90 ),
    QTransform().scale( 2, 2 ),
    QTransform().scale( 0.5, 0.5 ),
    QTransform().rotate( 45, Qt::YAxis ),
    QTransform().rotate( -45, Qt::YAxis ),
    QTransform().rotate( 45, Qt::XAxis ),
    QTransform().rotate( -45, Qt::XAxis ),
};

KDAB_UNITTEST_SIMPLE( KDTransformMapper, "kdtools/gui" ) {
#define DO( Type, samples ) \
    for ( unsigned int i = 0 ; i < sizeof samples / sizeof *samples ; ++i ) \
        for ( unsigned int j = 0 ; j < sizeof transforms / sizeof *transforms ; ++j ) \
            assertEqual( transforms[j].map( samples[i] ), KDTransformMapper( transforms[j] )( samples[i] ) )

    DO( QPoint,  points  );
    DO( QPointF, pointfs );
    DO( QLine,   lines   );
    DO( QLineF,  linefs  );

#undef DO

}

#endif // QT_VERSION >= 0x040300

#endif // KDTOOLSGUI_UNITTESTS
