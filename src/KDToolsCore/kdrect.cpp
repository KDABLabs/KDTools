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

#include "kdrect.h"

/*!
  \class KDPoint KDPoint
  \ingroup core
  \brief Point convenience functions

  KDPoint is a class that you can use instead of QPoint in cases where
  the missing constexpr support of QPoint is limiting you.

  The main use-case for this class is as a replacement for QPoint in
  constexpr KDRect functions.

  Please see KDRect for how to exploit the constexpr nature of KDPoint
  and KDRect.

  All functions in this class are reentrant.
*/

///\name Construction and Conversion
//@{

/*!
  \fn KDPoint::KDPoint()

  Default constructor.

  \post x() == 0
  \post y() == 0
*/

/*!
  \fn KDPoint::KDPoint( int x, int y )

  Constructs a point with x-value \a x and y-value \a y.

  \post x() == x
  \post y() == y
*/

/*!
  \fn KDPoint::KDPoint( const QPoint & p )

  Constructs a point as a copy of \a p.

  \post x() == p.x()
  \post y() == p.y()
*/

/*!
  \fn KDPoint & KDPoint::operator=( const QPoint & p )

  Implicit conversion assignment operator from QPoint.

  \post x() == p.x()
  \post y() == p.y()
*/

/*!
  \fn KDPoint::operator const QPoint & () const

  Implicit conversion to QPoint. This conversion is fast. It involves
  no copying of the bits.
*/

//@}

///\name Queries
//@{

/*!
  \fn int KDPoint::x() const

  Returns the x-coordinate of this point.

  \sa y()
*/

/*!
  \fn int KDPoint::y() const

  Returns the y-coordinate of this point.

  \sa x()
*/

//@}

/*!
  \fn KDPoint operator+( const KDPoint & lhs, const KDPoint & rhs )
  \relates KDPoint
  Adds \a lhs and \a rhs.
*/

/*!
  \fn KDPoint operator+( const QPoint & lhs, const KDPoint & rhs )
  \relates KDPoint
  \overload
*/

/*!
  \fn KDPoint operator+( const KDPoint & lhs, const QPoint & rhs )
  \relates KDPoint
  \overload
*/


/*!
  \fn KDPoint operator-( const KDPoint & lhs, const KDPoint & rhs )
  \relates KDPoint
  Subtracts \a rhs from \a lhs.
*/

/*!
  \fn KDPoint operator-( const QPoint & lhs, const KDPoint & rhs )
  \relates KDPoint
  \overload
*/

/*!
  \fn KDPoint operator-( const KDPoint & lhs, const QPoint & rhs )
  \relates KDPoint
  \overload
*/







/*!
  \class KDRect KDRect
  \ingroup core
  \brief Rectangle convenience functions

  KDRect is a class that you can use instead of QRect in most
  situations. It's designed around the idea that you can't have enough
  convenience methods on such a class, and that none of the member
  functions is non-const. It also features useful ways to construct
  rectangles that go beyond the simple two-point and point-and-size
  forms offered by QRect. KDRect is memory-compatible with QRect, so
  conversion of KDRect to QRect is a no-op. This allows you to use
  KDRect in your computations while not paying a price for the final
  conversion to QRect for drawing, but requires you to write slightly
  awkward code when working with arrays:
  \code
  static KDAB_DECL_CONSTEXPR_VAR KDRect rects[] = {
    KDRect::fromPoints( 0, 0, 12, 12 ),
    KDRect::fromPoints( 12, 12, 24, 24 ),
  }
  painter->drawRects( &static_cast<const QRect&>(rects[0]),
                      sizeof rects / sizeof *rects );
  \endcode

  All functions in this class that have a corresponding function on
  QRect of the same name, behave identically to their QRect
  counterparts, unless otherwise noted.

  All functions in this class are reentrant.

  \section constexpr constexpr

  Most KDRect operations support \c constexpr (with help from
  KDPoint). C++11 generalized constant expressions allow certain
  functions to run at compile-time. The immediate benefit is that
  \code
  static KDAB_DECL_CONSTEXPR_VAR KDRect rects[] = {
    KDRect::fromPoints( 0, 0, 12, 12 ),
    KDRect::fromPoints( 12, 12, 24, 24 ),
  };
  \endcode
  is evaluated at compile-time and placed into read-only memory by the
  linker, if the compiler supports \c constexpr. Contrast this with
  the C++03 situation:
  \code
  static const KDRect rects[] = {
    KDRect::fromPoints( 0, 0, 12, 12 ),
    KDRect::fromPoints( 12, 12, 24, 24 ),
  };
  \endcode
  which requires static initialisation, with all the problems
  involving initialisation order, and safety in the presence of
  multiple threads.
*/


///\name Construction and Conversion
//@{

/*!
  \fn KDRect::KDRect()

  Default constructor. Constructs an invalid KDRect.

  \post isValid() == false
  \post isEmpty() == true
  \post isNull() == true
  \sa isValid
*/

/*!
  \fn KDRect::KDRect( const QRect & rect )

  Implicit conversion constructor from QRect.

  \post *this == rect
  \sa fromRect
*/

/*!
  \fn KDRect & KDRect::operator=( const QRect & rect )

  Implicit conversion assignment opertor from QRect.

  \post *this == rect
  \sa fromRect
*/

/*!
  \fn KDRect::operator const QRect & () const

  Implicit conversion to QRect. This operation is fast. It involves no
  copying of the bits.
*/

/*!
  \fn KDRect KDRect::fromRect( const QRect & rect )

  Explicit conversion from QRect.

  \post *this == rect
  \sa KDRect( const QRect & rect )
*/

/*!
  \fn KDRect KDRect::fromPoints( int l, int t, int r, int b )

  Constructs a KDRect with top-left point (\a l, \a t) and
  bottom-right point (\a r, \a b).

  \post top() == t
  \post left() == l
  \post right() == r
  \post bottom() == b
*/

/*!
  \fn KDRect KDRect::fromPoints( const QPoint & tl, int r, int b )
  \overload
*/

/*!
  \fn KDRect KDRect::fromPoints( const KDPoint & tl, int r, int b )
  \overload
*/

/*!
  \fn KDRect KDRect::fromPoints( int l, int t, const QPoint & br )
  \overload
*/

/*!
  \fn KDRect KDRect::fromPoints( int l, int t, const KDPoint & br )
  \overload
*/

/*!
  \fn KDRect KDRect::fromPoints( const QPoint & tl, const QPoint & br )
  \overload
*/

/*!
  \fn KDRect KDRect::fromPoints( const KDPoint & tl, const KDPoint & br )
  \overload
*/

/*!
  \fn KDRect KDRect::fromTopLeftAndSize( int l, int t, int w, int h )

  Constructs a KDRect with top-left point (\a l, \a t), width \a w and
  height \a h.

  \post top() == t
  \post left() == l
  \post width() == w
  \post height() == h
*/

/*!
  \fn KDRect KDRect::fromTopLeftAndSize( int l, int t, const QSize & sz )
  \overload
*/

/*!
  \fn KDRect KDRect::fromTopLeftAndSize( const QPoint & tl, int w, int h )
  \overload
*/

/*!
  \fn KDRect KDRect::fromTopLeftAndSize( const KDPoint & tl, int w, int h )
  \overload
*/

/*!
  \fn KDRect KDRect::fromTopLeftAndSize( const QPoint & tl, const QSize & sz )
  \overload
*/

/*!
  \fn KDRect KDRect::fromTopRightAndSize( int r, int t, int w, int h )

  Constructs a KDRect with top-right point (\a r, \a t), width \a w and
  height \a h.

  \post top() == t
  \post right() == r
  \post width() == w
  \post height() == h
*/

/*!
  \fn KDRect KDRect::fromTopRightAndSize( int r, int t, const QSize & sz )
  \overload
*/

/*!
  \fn KDRect KDRect::fromTopRightAndSize( const QPoint & tr, int w, int h )
  \overload
*/

/*!
  \fn KDRect KDRect::fromTopRightAndSize( const KDPoint & tr, int w, int h )
  \overload
*/

/*!
  \fn KDRect KDRect::fromTopRightAndSize( const QPoint & tr, const QSize & sz )
  \overload
*/

/*!
  \fn KDRect KDRect::fromBottomLeftAndSize( int l, int b, int w, int h )

  Constructs a KDRect with bottom-left point (\a l, \a b), width \a w and
  height \a h.

  \post bottom() == b
  \post left() == l
  \post width() == w
  \post height() == h
*/

/*!
  \fn KDRect KDRect::fromBottomLeftAndSize( int l, int t, const QSize & sz )
  \overload
*/

/*!
  \fn KDRect KDRect::fromBottomLeftAndSize( const QPoint & bl, int w, int h )
  \overload
*/

/*!
  \fn KDRect KDRect::fromBottomLeftAndSize( const KDPoint & bl, int w, int h )
  \overload
*/

/*!
  \fn KDRect KDRect::fromBottomLeftAndSize( const QPoint & bl, const QSize & sz )
  \overload
*/

/*!
  \fn KDRect KDRect::fromBottomRightAndSize( int r, int b, int w, int h )

  Constructs a KDRect with bottom-right point (\a r, \a b), width \a w and
  height \a h.

  \post bottom() == b
  \post right() == r
  \post width() == w
  \post height() == h
*/

/*!
  \fn KDRect KDRect::fromBottomRightAndSize( int r, int b, const QSize & sz )
  \overload
*/

/*!
  \fn KDRect KDRect::fromBottomRightAndSize( const QPoint & br, int w, int h )
  \overload
*/

/*!
  \fn KDRect KDRect::fromBottomRightAndSize( const KDPoint & br, int w, int h )
  \overload
*/

/*!
  \fn KDRect KDRect::fromBottomRightAndSize( const QPoint & br, const QSize & sz )
  \overload
*/

/*!
  \fn KDRect KDRect::fromCenterPointAndSize( int x, int y, int w, int h )

  Constructs a KDRect with center point (\a x, \a y), width \a w and
  height \a h.

  \post hCenter() == x
  \post vCenter() == y
  \post width() == w
  \post height() == h
*/

/*!
  \fn KDRect KDRect::fromCenterPointAndSize( int x, int y, const QSize & sz )
  \overload
*/

/*!
  \fn KDRect KDRect::fromCenterPointAndSize( const QPoint & c, int w, int h )
  \overload
*/

/*!
  \fn KDRect KDRect::fromCenterPointAndSize( const KDPoint & c, int w, int h )
  \overload
*/

/*!
  \fn KDRect KDRect::fromCenterPointAndSize( const QPoint & c, const QSize & sz )
  \overload
*/

//@}



///\name Queries
//@{

/*!
  \fn bool KDRect::isNull() const
  \return width() == 0 && height() == 0
  \sa QRect::isNull
*/

/*!
  \fn bool KDRect::isEmpty() const
  \return width() <= 0 || height() <= 0
  \sa QRect::isEmpty
*/

/*!
  \fn bool KDRect::isValid() const
  \return width() > 0 && height() > 0
  \sa QRect::isValid
*/

/*!
  \fn bool KDRect::contains( int x, int y ) const

  \returns \a true if point (\a x,\a y) is contained in this
  rectangle.

  Egde and corner points are considered part of the rectangles for
  this test.

  If this rectangle is invalid, \c false is returned.

  \sa QRect::contains
*/

/*!
  \fn bool KDRect::contains( const QPoint & p ) const
  \overload
*/

/*!
  \fn bool KDRect::contains( const KDPoint & p ) const
  \overload
*/

/*!
  \fn bool KDRect::contains( const QRect & rect ) const
  \returns contains( rect.topLeft() ) && contains( rect.bottomRight() )
*/

/*!
  \fn bool KDRect::contains( const KDRect & rect ) const
  \overload
*/

/*!
  \fn bool KDRect::intersects( const QRect & rect ) const
  \returns \c true if this rectangle and \a rect intersect.

  Edge and corner points are considered part of the rectangles for
  this test. If this rectangle or \a rect are invalid, \c false is
  returned.

  \sa QRect::intersects
*/

/*!
  \overload
*/
bool KDRect::intersects( const KDRect & rect ) const {
    return
        isValid()                &&
        rect.isValid()           &&
        left()   <= rect.left()  &&
        top()    <= rect.top()   &&
        right()  >= rect.right() &&
        bottom() >= rect.bottom();
}

static bool intersects( const QLine & l1, const QLine & l2 ) {

    if ( l1.isNull() || l2.isNull() )
        return l1.p1() == l2.p1() ;

    // this implementation follows
    // http://local.wasp.uwa.edu.au/~pbourke/geometry/lineline2d/

    const int x1 = l1.p1().x(), x2 = l1.p2().x(), x3 = l2.p1().x(), x4 = l2.p2().x() ;
    const int y1 = l1.p1().y(), y2 = l1.p2().y(), y3 = l2.p1().y(), y4 = l2.p2().y() ;

    const int denom  = ( y4 - y3 ) * ( x2 - x1 ) - ( x4 - x3 ) * ( y2 - y1 ) ;
    const int l1_nom = ( x4 - x3 ) * ( y1 - y3 ) - ( y4 - y3 ) * ( x1 - x3 ) ;
    const int l2_nom = ( x2 - x1 ) * ( y1 - y3 ) - ( y2 - y1 ) * ( x1 - x3 ) ;

    // 0 <= ( l{1,2}_nom / (double)denom ) <= 1  --> intersects

    if ( denom == 0 ) // parallel
        return l1_nom == 0 // coincident...
            && ( qMax( x1, x2 ) <= qMin( x3, x4 ) || qMin( x1, x2 ) <= qMax( x3, x4 ) )
            && ( qMax( y1, y2 ) <= qMin( y3, y4 ) || qMin( y1, y2 ) <= qMax( y3, y4 ) )
            && ( qMin( x1, x2 ) >= qMax( x3, x4 ) || qMax( x1, x2 ) >= qMin( x3, x4 ) )
            && ( qMin( y1, y2 ) >= qMax( y3, y4 ) || qMax( y1, y2 ) >= qMin( y3, y4 ) ) ;
   
    const int ua = l1_nom / denom;
    const int ub = l2_nom / denom;

    if( denom > 0 )
        return l1_nom > 0 && l2_nom > 0
            && ( denom == l1_nom || ua == 0 ) // 0 <= nom/denom <= 1
            && ( denom == l2_nom || ub == 0 );
    else    
        return l1_nom <= 0 && l2_nom <= 0
            && ( denom == l1_nom || ua == 0 ) // 0 <= nom/denom <= 1
            && ( denom == l2_nom || ub == 0 );
}

/*!
  \returns \c true if \a line intersects this rectangle.

  Edge and corner points are considered part of the rectangles for
  this test. If this rectangle is invalid, \c false is returned.
*/
bool KDRect::intersects( const QLine & line ) const {
    // a line intersects a rectangle if any of its endpoints is
    // contained in the rectangle or if it intersects one of it's edges:
    return
        contains( line.p1() )            ||
        contains( line.p2() )            ||
        ::intersects( topEdge(),    line ) ||
        ::intersects( leftEdge(),   line ) ||
        ::intersects( rightEdge(),  line ) ||
        ::intersects( bottomEdge(), line ) ;
}

/*!
  \fn int KDRect::left() const
  \returns the x-coordinate of the left border.
  \sa right top bottom hCenter vCenter leftMoved
*/

/*!
  \fn int KDRect::right() const
  \returns the x-coordinate of the right border.
  \inv right() == left() + width() - 1
  \sa left top bottom hCenter vCenter rightMoved
*/

/*!
  \fn int KDRect::top() const
  \returns the y-coordinate of the top border.
  \sa bottom left right hCenter vCenter topMoved
*/

/*!
  \fn int KDRect::bottom() const
  \returns the y-coordinate of the bottom border.
  \sa top left right hCenter vCenter bottomMoved
*/

/*!
  \fn int KDRect::hCenter() const
  \returns the x-coordinate of the center point.
  \inv hCenter() == ( left() + right() ) / 2
  \sa vCenter left right top bottom hCenterMoved
*/

/*!
  \fn int KDRect::vCenter() const
  \returns the y-coordinate of the center point.
  \inv vCenter() == ( top() + bottom() ) / 2
  \sa hCenter left right top bottom vCenterMoved
*/

/*!
  \fn int KDRect::area() const
  \return the area of this rectangle in pixels
  \inv area() == height() * width()
*/

/*!
  \fn int KDRect::height() const
  \returns the height of the rectangle in pixels
  \inv height() == bottom() - top() + 1
  \sa width size
*/

/*!
  \fn int KDRect::width() const
  \returns the width of the rectangle in pixels
  \inv width() == right() - left() + 1
  \sa height size
*/

/*!
  \fn QSize KDRect::size() const
  \returns the size of the rectangle in pixels
  \inv size() == QSize( width(), height() )
  \sa width height
*/

/*!
  \fn KDPoint KDRect::topLeft() const
  \returns the coordinates of the top-left corner of the rectangle,
  ie. (left(),top())
*/

/*!
  \fn KDPoint KDRect::topCenter() const
  \returns the coordinates of the middle point of the top edge of the
  rectangle, ie. (hCenter(),top())
*/

/*!
  \fn KDPoint KDRect::topRight() const
  \returns the coordinates of the top-right corner of the rectangle,
  ie. (right(),top())
*/

/*!
  \fn KDPoint KDRect::leftCenter() const

  \returns the coordinates of the middle point of the left edge of the
  rectangle, ie. (left(),vCenter())
*/

/*!
  \fn KDPoint KDRect::center() const
  \returns the coordinates of the middle point of the rectangle,
  ie. (hCenter(),vCenter())
*/

/*!
  \fn KDPoint KDRect::rightCenter() const
  \returns the coordinates of the middle point of the right edge of
  the rectangle, ie. (right(),top())
*/

/*!
  \fn KDPoint KDRect::bottomLeft() const
  \returns the coordinates of the bottom-left corner of the rectangle,
  ie. (left(),bottom())
*/

/*!
  \fn KDPoint KDRect::bottomCenter() const
  \returns the coordinates of the middle point of the bottom edge of the
  rectangle, ie. (hCenter(),bottom())
*/

/*!
  \fn KDPoint KDRect::bottomRight() const
  \returns the coordinates of the bottom-right corner of the rectangle,
  ie. (right(),bottom())
*/

/*!
  \fn KDPoint KDRect::referencePoint( Qt::Alignment align ) const

  \return the coordinates of the reference point descripbed by \a
  align of the rectangle.

  The interpretation of \a align is as follows:
  <table>
  <tr><th></th><th>Qt::AlignLeft</th><th>Qt::AlignHCenter</th><th>Qt::AlignRight</th></tr>
  <tr><th>Qt::AlignTop</th><td>topLeft()</td><td>topCenter()</td><td>topRight()</td></tr>
  <tr><th>Qt::AlignVCenter</th><td>leftCenter()</td><td>center()</td><td>rightCenter()</td></tr>
  <tr><th>Qt::AlignBottom</th><td>bottomLeft()</td><td>bottomCenter()</td><td>bottomRight()</td></tr>
  </table>
*/

/*!
  \fn KDPoint KDRect::referencePoint( Qt::Corner corner ) const
  \overload
*/

/*!
  \fn QLine KDRect::topEdge() const
  \return the top edge of the rectangle
  \inv topEdge() == QLine(topLeft(),topRight())
*/

/*!
  \fn QLine KDRect::leftEdge() const
  \return the left edge of the rectangle
  \inv leftEdge() == QLine(topLeft(),bottomLeft())
*/

/*!
  \fn QLine KDRect::rightEdge() const
  \return the right edge of the rectangle
  \inv rightEdge() == QLine(topRight(),bottomRight())
*/

/*!
  \fn QLine KDRect::bottomEdge() const
  \return the bottom edge of the rectangle
  \inv bottomEdge() == QLine(bottomLeft(),bottomRight())
*/

/*!
  \fn QLine KDRect::hCenterLine() const
  \return the horizontal line through the center of the rectangle
  \inv hCenterLine() == QLine(leftCenter(),rightCenter())
*/

/*!
  \fn QLine KDRect::vCenterLine() const
  \return the vertical line through the center
  \inv vCenterLine() == QLine(topCenter(),bottomCenter())
*/

/*!
  \fn QLine KDRect::topLeftDiagonal() const
  \return the diagonal that starts in the top-left corner and ends in
  the bottom-right corner
  \inv topLeftDiagonal() == QLine(topLeft(),bottomRight())
*/

/*!
  \fn QLine KDRect::bottomLeftDiagonal() const

  \return he diagonal that starts in the bottom-left corner and ends
  in the to-right corner
  \inv bottomLeftDiagonal() == QLine(bottomLeft(),topRight())
*/

//@}

///\name Modifiers
//@{

/*!
  \fn KDRect KDRect::normalized() const

  \return A rectangle which covers the same area, but non-negative
  width() and height()

  \code
  // Example:
  const KDRect r = KDRect::fromPoints( 10, 10, 0, 0 );
  r.normalized(); // == fromPoints( 0, 0, 10, 10 )
  \endcode

  \inv abs(width())
  \inv abs(height())
  \inv isNull()
  \post width() >= 0
  \post height() >= 0

  \sa isValid()
*/

/*!
  \fn KDRect KDRect::intersected( const KDRect & rect ) const

  \return the largest rectangle contained in both \a rect and this
  rectangle, ie. the intersection of both.

  \pre isValid() || isNull()
  \pre rect.isValid() || rect.isNull()
  \post this->contains( retval ) == true
  \post rect.contains( retval ) == true

  \sa united
*/

/*!
  \fn KDRect KDRect::intersected( const QRect & rect ) const
  \overload
*/

/*!
  \fn KDRect KDRect::united( const KDRect & rect ) const

  \return the smallest rectangle containing both \a rect and this
  rectangle, ie. the union of both.

  \pre isValid() || isNull()
  \pre rect.isValid() || rect.isNull()
  \post retval->contains( this ) == true
  \post retval->contains( rect ) == true

  \sa intersected
*/

/*!
  \fn KDRect KDRect::united( const QRect & rect ) const
  \overload
*/

/*!
  \fn KDRect KDRect::united( int x, int y ) const

  \return the smallest rectangle containing both this rectangle and
  the point (\a x, \a y).

  \pre isValid() || isNull()
  \post retval->contains( this ) == true
  \post retval->contains( x, y ) == true

  \sa intersected
*/

/*!
  \fn KDRect KDRect::united( const QPoint & p ) const
  \overload
*/

/*!
  \fn KDRect KDRect::united( const KDPoint & p ) const
  \overload
*/

/*!
  \fn KDRect KDRect::movedBy( int dx, int dy ) const

  \return this rectangle offset by (\a dx, \a dy).

  \post retval->topLeft() == this->topLeft() + QPoint( dx, dy )

  \sa translated
*/

/*!
  \fn KDRect KDRect::movedBy( const QPoint & p ) const
  \overload
*/

/*!
  \fn KDRect KDRect::movedBy( const KDPoint & p ) const
  \overload
*/

/*!
  \fn KDRect KDRect::translated( int dx, int dy ) const

  Equivalent to movedBy().
*/

/*!
  \fn KDRect KDRect::translated( const QPoint & p ) const
  \overload
*/

/*!
  \fn KDRect KDRect::translated( const KDPoint & p ) const
  \overload
*/

/*!
  \fn KDRect KDRect::movedTo( int l, int t ) const

  Equivalent to topLefeMoved().
*/

/*!
  \fn KDRect KDRect::movedTo( const QPoint & tl ) const
  \overload
*/

/*!
  \fn KDRect KDRect::movedTo( const KDPoint & tl ) const
  \overload
*/

/*!
  \fn KDRect KDRect::topMoved( int t ) const

  Moves the top edge of this rectangle to y-coordinate \a t while
  leaving height and width unchanged.
*/

/*!
  \fn KDRect KDRect::leftMoved( int l ) const

  Moves the left edge of this rectangle to x-coordinate \a l while
  leaving height and width unchanged.
*/

/*!
  \fn KDRect KDRect::rightMoved( int r ) const

  Moves the right edge of this rectangle to x-coordinate \a r while
  leaving height and width unchanged.
*/

/*!
  \fn KDRect KDRect::bottomMoved( int b ) const

  Moves the bottom edge of this rectangle to y-coordinate \a b while
  leaving height and width unchanged.
*/

/*!
  \fn KDRect KDRect::vCenterMoved( int v ) const

  Moves the y-coordinate of the center of this rectangle to \a v while
  leaving height and width unchanged.
*/

/*!
  \fn KDRect KDRect::hCenterMoved( int h ) const

  Moves the x-coordinate of the center of this rectangle to \a h while
  leaving height and width unchanged.
*/

/*!
  \fn KDRect KDRect::referencePointMoved( Qt::Alignment align, int x, int y ) const

  Moves the reference point described by \a align to (\a x, \a y)
  while leaving height and width unchanged.

  See the documentation for the referencePoint() member for how \a
  align is interpreted.
*/

/*!
  \fn KDRect KDRect::referencePointMoved( Qt::Alignment align, const QPoint & p ) const
  \overload
*/

/*!
  \fn KDRect KDRect::referencePointMoved( Qt::Alignment align, const KDPoint & p ) const
  \overload
*/

/*!
  \fn KDRect KDRect::referencePointMoved( Qt::Corner corner, int x, int y ) const
  \overload
*/

/*!
  \fn KDRect KDRect::referencePointMoved( Qt::Corner corner, const QPoint & p ) const
  \overload
*/

/*!
  \fn KDRect KDRect::referencePointMoved( Qt::Corner corner, const KDPoint & p ) const
  \overload
*/

/*!
  \fn KDRect KDRect::topLeftMoved( int l, int t ) const

  Moves the top-left point of this rectangle to (\a l, \a t) while
  keeping width and height unchanged.

  Equivalent to referencePointMoved( Qt::AlignTop|Qt::AlignLeft, l, t )
*/

/*!
  \fn KDRect KDRect::topLeftMoved( const QPoint & tl ) const
  \overload
*/

/*!
  \fn KDRect KDRect::topLeftMoved( const KDPoint & tl ) const
  \overload
*/

/*!
  \fn KDRect KDRect::topCenterMoved( int c, int t ) const

  Moves the top-center point of this rectangle to (\a c, \a t) while
  keeping width and height unchanged.

  Equivalent to referencePointMoved( Qt::AlignTop|Qt::AlignHCenter, c, t )
*/

/*!
  \fn KDRect KDRect::topCenterMoved( const QPoint & tc ) const
  \overload
*/

/*!
  \fn KDRect KDRect::topCenterMoved( const KDPoint & tc ) const
  \overload
*/

/*!
  \fn KDRect KDRect::topRightMoved( int r, int t ) const

  Moves the top-right point of this rectangle to (\a r, \a t) while
  keeping width and height unchanged.

  Equivalent to referencePointMoved( Qt::AlignTop|Qt::AlignRight, r, t )
*/

/*!
  \fn KDRect KDRect::topRightMoved( const QPoint & tr ) const
  \overload
*/

/*!
  \fn KDRect KDRect::topRightMoved( const KDPoint & tr ) const
  \overload
*/

/*!
  \fn KDRect KDRect::leftCenterMoved( int l, int c ) const

  Moves the left-center point of this rectangle to (\a l, \a c) while
  keeping width and height unchanged.

  Equivalent to referencePointMoved( Qt::AlignVCenter|Qt::AlignLeft, l, c )
*/

/*!
  \fn KDRect KDRect::leftCenterMoved( const QPoint & lc ) const
  \overload
*/

/*!
  \fn KDRect KDRect::leftCenterMoved( const KDPoint & lc ) const
  \overload
*/

/*!
  \fn KDRect KDRect::centerMoved( int x, int y ) const

  Moves the center point of this rectangle to (\a x, \a y) while
  keeping width and height unchanged.

  Equivalent to referencePointMoved( Qt::AlignCenter, x, y )
*/

/*!
  \fn KDRect KDRect::centerMoved( const QPoint & c ) const
  \overload
*/

/*!
  \fn KDRect KDRect::centerMoved( const KDPoint & c ) const
  \overload
*/

/*!
  \fn KDRect KDRect::rightCenterMoved( int r, int c ) const

  Moves the right-center point of this rectangle to (\a r, \a c) while
  keeping width and height unchanged.

  Equivalent to referencePointMoved( Qt::AlignVCenter|Qt::AlignRight, r, c )
*/

/*!
  \fn KDRect KDRect::rightCenterMoved( const QPoint & rc ) const
  \overload
*/

/*!
  \fn KDRect KDRect::rightCenterMoved( const KDPoint & rc ) const
  \overload
*/

/*!
  \fn KDRect KDRect::bottomLeftMoved( int l, int b ) const

  Moves the bottom-left point of this rectangle to (\a l, \a b) while
  keeping width and height unchanged.

  Equivalent to referencePointMoved( Qt::AlignBottom|Qt::AlignLeft, l, b )
*/

/*!
  \fn KDRect KDRect::bottomLeftMoved( const QPoint & bl ) const
  \overload
*/

/*!
  \fn KDRect KDRect::bottomLeftMoved( const KDPoint & bl ) const
  \overload
*/

/*!
  \fn KDRect KDRect::bottomCenterMoved( int c, int b ) const

  Moves the bottom-center point of this rectangle to (\a c, \a b) while
  keeping width and height unchanged.

  Equivalent to referencePointMoved( Qt::AlignBottom|Qt::AlignHCenter, c, b )
*/

/*!
  \fn KDRect KDRect::bottomCenterMoved( const QPoint & bc ) const
  \overload
*/

/*!
  \fn KDRect KDRect::bottomCenterMoved( const KDPoint & bc ) const
  \overload
*/

/*!
  \fn KDRect KDRect::bottomRightMoved( int r, int b ) const

  Moves the bottom-right point of this rectangle to (\a r, \a b) while
  keeping width and height unchanged.

  Equivalent to referencePointMoved( Qt::AlignBottom|Qt::AlignRight, r, b )
*/

/*!
  \fn KDRect KDRect::bottomRightMoved( const QPoint & br ) const
  \overload
*/

/*!
  \fn KDRect KDRect::bottomRightMoved( const KDPoint & br ) const
  \overload
*/

/*!
  \fn KDRect KDRect::aligned( const KDRect & ref, Qt::Alignment align ) const

  \returns this rectangle moved in such a way as to align it according
  to \a align inside \a ref while keeping width and height unchanged.

  Examples:
  \code
  const KDRect ref = KDRect::fromPoints( 0,0, 100,100 );
  const KDRect rect = KDRect::fromPoints( 10,10, 20,20 );

  rect.aligned( ref, Qt::AlignTop ) // ( 10,0, 20,10 )
  rect.algined( ref, Qt::AlignBottom|Qt::AlignRight ) // ( 90,90, 100,100 )
  \endcode
*/

/*!
  \fn KDRect KDRect::aligned( const KDRect & ref, Qt::Corner corner ) const
  \overload
*/

/*!
  \fn KDRect KDRect::aligned( const QRect & ref, Qt::Alignment align ) const
  \overload
*/

/*!
  \fn KDRect KDRect::aligned( const QRect & ref, Qt::Corner corner ) const
  \overload
*/

/*!
  \fn KDRect KDRect::resized( int w, int h ) const

  \returns this rectangle resized to width \a w and height \a h while.

  \pre w >= 0
  \pre h >= 0
  \inv topLeft()
*/

/*!
  \fn KDRect KDRect::resized( const QSize & sz ) const
  \overload
*/

/*!
  \fn KDRect KDRect::transposed() const

  \returns a rectangle resized such that it's former height becomes
  it's width and vice versa.

  Equivalent to resize( height(), width() )

  \inv topLeft()
  \inv area()
*/

/*!
  \fn KDRect KDRect::mirrored( int x, int y ) const

  \returns this rectangle point-mirrored at (\a x, \a y ).

  \inv width()
  \inv height()
*/

/*!
  \fn KDRect KDRect::mirrored( const QPoint & p ) const
  \overload
*/

/*!
  \fn KDRect KDRect::mirrored( const KDPoint & p ) const
  \overload
*/

/*!
  \fn KDRect KDRect::mirrored() const
  \overload

  Equivalent to mirrored( 0, 0 ).
*/

/*!
  \fn KDRect KDRect::adjusted( int dl, int dt, int dr, int db ) const

  \returns this rectable with each of its edges x adjusted by
  resp. value dx (additive).
*/

/*!
  \fn KDRect KDRect::grown( int delta ) const

  \returns this rectangle grown on all sides by \a delta.

  \inv center()
*/

/*!
  \fn KDRect KDRect::shrunken( int delta ) const

  \returns this rectangle shrunken on all sides by \a delta.

  \inv center()
*/

/*!
  \fn KDRect KDRect::expandedTo( int w, int h ) const

  \returns this rectangle with it's width expanded to \a w and it's
  height expanded to \a h.

  \inv topLeft()
  \post width() >= w
  \post height() >= h

  \sa QSize::expandedTo()
*/

/*!
  \fn KDRect KDRect::expandedTo( const QSize & sz ) const
  \overload
*/

/*!
  \fn KDRect KDRect::boundedTo( int w, int h ) const

  \returns this rectangle with it's width bounded to \a w and it's
  height bounded to \a h.

  \inv topLeft()
  \post width() <= w
  \port height() <= h

  \sa QSize::boundedTo()
*/

/*!
  \fn KDRect KDRect::boundedTo( const QSize & sz ) const
  \overload
*/


//@}

/*!
  \fn bool operator==( const KDRect & lhs, const KDRect & rhs )
  \relates KDRect
  \returns whether \a lhs and \a rhs are equal
*/

/*!
  \fn bool operator==( const KDRect & lhs, const QRect & rhs )
  \relates KDRect
  \overload
*/

/*!
  \fn bool operator==( const QRect & lhs, const KDRect & rhs )
  \relates KDRect
  \overload
*/

/*!
  \fn bool operator!=( const KDRect & lhs, const KDRect & rhs )
  \relates KDRect
  \returns whether \a lhs and \a rhs are unequal
*/

/*!
  \fn bool operator!=( const KDRect & lhs, const QRect & rhs )
  \relates KDRect
  \overload
*/

/*!
  \fn bool operator!=( const QRect & lhs, const KDRect & rhs )
  \relates KDRect
  \overload
*/

/*!
  \fn KDRect KDRect::operator&( const KDRect & lhs, const KDRect & rhs )
  \relates KDRect
  \returns the intersection of \a lhs and \a rhs
  \sa intersected()
*/

/*!
  \fn KDRect KDRect::operator&( const QRect & lhs, const KDRect & rhs )
  \relates KDRect
  \overload
*/

/*!
  \fn KDRect KDRect::operator&( const KDRect & lhs, const QRect & rhs )
  \relates KDRect
  \overload
*/

/*!
  \fn KDRect KDRect::operator|( const KDRect & lhs, const KDRect & rhs )
  \relates KDRect
  \returns the union of \a lhs and \a rhs
  \sa united()
*/

/*!
  \fn KDRect KDRect::operator|( const QRect & lhs, const KDRect & rhs )
  \relates KDRect
  \overload
*/

/*!
  \fn KDRect KDRect::operator|( const KDRect & lhs, const QRect & rhs )
  \relates KDRect
  \overload
*/

/*!
  \fn KDRect KDRect::operator|( const KDRect & lhs, const QPoint & rhs )
  \relates KDRect
  \overload
*/

/*!
  \fn KDRect KDRect::operator|( const KDRect & lhs, const KDPoint & rhs )
  \relates KDRect
  \overload
*/

/*!
  \fn KDRect KDRect::operator|( const QPoint & lhs, const KDRect & rhs )
  \relates KDRect
  \overload
*/

/*!
  \fn KDRect KDRect::operator|( const KDPoint & lhs, const KDRect & rhs )
  \relates KDRect
  \overload
*/

/*!
  \fn KDRect KDRect::operator|( const QRect & lhs, const QPoint & rhs )
  \relates KDRect
  \overload
*/

/*!
  \fn KDRect KDRect::operator|( const QRect & lhs, const KDPoint & rhs )
  \relates KDRect
  \overload
*/

/*!
  \fn KDRect KDRect::operator|( const QPoint & lhs, const QRect & rhs )
  \relates KDRect
  \overload
*/

/*!
  \fn KDRect KDRect::operator|( const KDPoint & lhs, const QRect & rhs )
  \relates KDRect
  \overload
*/


#ifdef KDTOOLSCORE_UNITTESTS

#include <KDUnitTest/Test>
//#include <QtGui/QPainter>

QT_BEGIN_NAMESPACE
static inline std::ostream& operator<<( std::ostream& stream, const QPoint& point )
{
    stream << "QPoint( " << point.x() << ", " << point.y() << " )";
    return stream;
}

static inline std::ostream& operator<<( std::ostream& stream, const QLine& line )
{
    stream << "QLine( " << line.p1() << ", " << line.p2() << " )";
    return stream;
}
QT_END_NAMESPACE

KDAB_UNITTEST_SIMPLE( KDRect, "kdtools/core" ) {

    const KDRect rect = KDRect::fromPoints( 0, 0, 10, 10 );

    assertTrue( rect.intersects( rect.topEdge() ) );
    assertTrue( rect.intersects( rect.bottomEdge() ) );
    assertTrue( rect.intersects( rect.leftEdge() ) );
    assertTrue( rect.intersects( rect.rightEdge() ) );

    assertTrue( rect.intersects( QLine( rect.topLeft(), QPoint( -20, -10 ) ) ) );
    assertTrue( rect.intersects( QLine( rect.bottomRight(), QPoint( 20, 10 ) ) ) );

    assertEqual( rect.topLeft(), QPoint( 0, 0 ) );
    assertEqual( rect.topRight(), QPoint( 10, 0 ) );
    assertEqual( rect.bottomLeft(), QPoint( 0, 10 ) );
    assertEqual( rect.bottomRight(), QPoint( 10, 10 ) );

    assertEqual( rect.topEdge(), QLine( 0, 0, 10, 0 ) );
    assertEqual( rect.bottomEdge(), QLine( 0, 10, 10, 10 ) );
    assertEqual( rect.leftEdge(), QLine( 0, 0, 0, 10 ) );
    assertEqual( rect.rightEdge(), QLine( 10, 0, 10, 10 ) );

    //
    // PARALLEL LINES
    //
    // outside of rectangle:
    assertFalse( rect.contains( QPoint( -1, 0 ) ) ); // left
    assertFalse( rect.contains( QPoint( -1, 10 ) ) ); // left
    assertFalse( intersects( rect.topEdge(), QLine( -1, 0, -1, 10 ) ) );
    assertFalse( intersects( rect.leftEdge(), QLine( -1, 0, -1, 10 ) ) );
    assertFalse( intersects( rect.rightEdge(), QLine( -1, 0, -1, 10 ) ) );
    assertFalse( intersects( rect.bottomEdge(), QLine( -1, 0, -1, 10 ) ) );
    assertFalse( rect.intersects( QLine( -1,  0,  -1, 10 ) ) ); // left
    
    assertFalse( rect.intersects( QLine(  0, -1,  10, -1 ) ) ); // bottom
    assertFalse( rect.intersects( QLine( 11,  0,  11, 10 ) ) ); // top
    assertFalse( rect.intersects( QLine(  0, 11,  10, 11 ) ) ); // right
    // edges:
    assertTrue( rect.intersects( QLine(  0,  0,   0, 10 ) ) ); // left
    assertTrue( rect.intersects( QLine(  0,  0,  10,  0 ) ) ); // bottom
    assertTrue( rect.intersects( QLine(  0, 10,  10, 10 ) ) ); // top
    assertTrue( rect.intersects( QLine( 10,  0,  10, 10 ) ) ); // right
    // fully inside of rectangle:
    assertTrue( rect.intersects( QLine( 1, 1,  1, 9 ) ) ); // left
    assertTrue( rect.intersects( QLine( 1, 1,  9, 1 ) ) ); // bottom
    assertTrue( rect.intersects( QLine( 1, 9,  9, 9 ) ) ); // top
    assertTrue( rect.intersects( QLine( 9, 1,  9, 9 ) ) ); // right
    // partially inside of rectangle:
    assertTrue( rect.intersects( QLine(  1, -1,   1, 11 ) ) ); // left
    assertTrue( rect.intersects( QLine( -1,  1,  11,  1 ) ) ); // bottom
    assertTrue( rect.intersects( QLine( -1,  9,  11,  9 ) ) ); // top
    assertTrue( rect.intersects( QLine(  9, -1,   9, 11 ) ) ); // right
    // w/exactly one endpoint inside:
    assertTrue( rect.intersects( QLine( 5, 5,  -1,  5 ) ) ); // west
    assertTrue( rect.intersects( QLine( 5, 5,   5, -1 ) ) ); // south
    assertTrue( rect.intersects( QLine( 5, 5,   5, 11 ) ) ); // north
    assertTrue( rect.intersects( QLine( 5, 5,  11,  5 ) ) ); // east
    // touching an egde:
    assertTrue( rect.intersects( QLine( -5,  5,   0,  5 ) ) ); // west
    assertTrue( rect.intersects( QLine(  5, -5,   5,  0 ) ) ); // south
    assertTrue( rect.intersects( QLine(  5, 15,   5, 10 ) ) ); // north
    assertTrue( rect.intersects( QLine( 15,  5,  10,  5 ) ) ); // east
    // touching a corner:
    assertTrue( rect.intersects( QLine(  0,  0,  -5,  0 ) ) ); // SW-west
    assertTrue( rect.intersects( QLine(  0,  0,   0, -5 ) ) ); // SW-south
    assertTrue( rect.intersects( QLine( -5,  5,   5, -5 ) ) ); // SW-diagonal
    assertTrue( rect.intersects( QLine(  0, 10,  -5, 10 ) ) ); // NW-west
    assertTrue( rect.intersects( QLine(  0, 10,   0, 15 ) ) ); // NW-north
    assertTrue( rect.intersects( QLine( -5,  5,   5, 15 ) ) ); // NW-diagonal
    assertTrue( rect.intersects( QLine( 10,  0,  15,  0 ) ) ); // SE-east
    assertTrue( rect.intersects( QLine( 10,  0,  10, -5 ) ) ); // SE-south
    assertTrue( rect.intersects( QLine(  5, -5,  15,  5 ) ) ); // SE-diagonal
    assertTrue( rect.intersects( QLine( 10, 10,  15, 10 ) ) ); // NE-east
    assertTrue( rect.intersects( QLine( 10, 10,  10, 15 ) ) ); // NE-north
    assertTrue( rect.intersects( QLine(  5, 15,  15,  5 ) ) ); // NE-diagonal
    // almost touching an edge:
    assertFalse( rect.intersects( QLine( -5,  5,  -1,  5 ) ) ); // west
    assertFalse( rect.intersects( QLine(  5, -5,   5, -1 ) ) ); // south
    assertFalse( rect.intersects( QLine(  5, 15,   5, 11 ) ) ); // north
    assertFalse( rect.intersects( QLine( 15,  5,  11,  5 ) ) ); // east

    //
    // DEGENERATE LINES (iow: start==end)
    //
    // inside
    assertTrue( rect.intersects( QLine(  5,  5,   5,  5 ) ) ); // middle
    assertTrue( rect.intersects( QLine(  0,  5,   0,  5 ) ) ); // edge L
    assertTrue( rect.intersects( QLine(  5,  0,   5,  0 ) ) ); // edge B
    assertTrue( rect.intersects( QLine( 10,  5,  10,  5 ) ) ); // edge R
    assertTrue( rect.intersects( QLine(  5, 10,   5, 10 ) ) ); // edge T
    assertTrue( rect.intersects( QLine( 10, 10,  10, 10 ) ) ); // corner NE
    assertTrue( rect.intersects( QLine( 10,  0,  10,  0 ) ) ); // corner SE
    assertTrue( rect.intersects( QLine(  0, 10,   0, 10 ) ) ); // corner NW
    assertTrue( rect.intersects( QLine(  0,  0,   0,  0 ) ) ); // corner SW
    // outside
    assertFalse( rect.intersects( QLine( -1,  5,  -1,  5 ) ) ); // edge L
    assertFalse( rect.intersects( QLine(  5, -1,   5, -1 ) ) ); // edge B
    assertFalse( rect.intersects( QLine( 11,  5,  11,  5 ) ) ); // edge R
    assertFalse( rect.intersects( QLine(  5, 11,   5, 11 ) ) ); // edge T
    assertFalse( rect.intersects( QLine( 11, 10,  11, 10 ) ) ); // corner NE east
    assertFalse( rect.intersects( QLine( 10, 11,  10, 11 ) ) ); // corner NE north
    assertFalse( rect.intersects( QLine( 11, 11,  11, 11 ) ) ); // corner NE ne
    assertFalse( rect.intersects( QLine( 11,  0,  11,  0 ) ) ); // corner SE east
    assertFalse( rect.intersects( QLine( 10, -1,  10, -1 ) ) ); // corner SE south
    assertFalse( rect.intersects( QLine( 11, -1,  11, -1 ) ) ); // corner SE se
    assertFalse( rect.intersects( QLine( -1, 10,  -1, 10 ) ) ); // corner NW west
    assertFalse( rect.intersects( QLine(  0, 11,   0, 11 ) ) ); // corner NW north
    assertFalse( rect.intersects( QLine( -1, 11,  -1, 11 ) ) ); // corner NW nw
    assertFalse( rect.intersects( QLine( -1,  0,  -1,  0 ) ) ); // corner SW
    assertFalse( rect.intersects( QLine(  0, -1,   0, -1 ) ) ); // corner SW
    assertFalse( rect.intersects( QLine( -1, -1,  -1, -1 ) ) ); // corner SW
		     

    assertFalse( rect.intersects( QLine( -8, 2,  2, 14 ) ) );
    assertTrue( rect.intersects( QLine( -5, 3,  15, 9 ) ) );
    
    assertTrue( rect.intersects( QLine( 0, 10,  10, 10 ) ) );
    assertFalse( rect.intersects( QLine( -1, 10,  -1, -10 ) ) );

    // coincident lines
    const KDRect rect2 = KDRect::fromTopLeftAndSize( 110, 2, 36, 36 );
    // to the top edge
    assertFalse( intersects( rect2.topEdge(), QLine( 150, 2, 160, 2 ) ) );
    assertFalse( intersects( rect2.topEdge(), QLine(  10, 2,  20, 2 ) ) );
    assertTrue(  intersects( rect2.topEdge(), QLine(  10, 2, 160, 2 ) ) );
    assertTrue(  intersects( rect2.topEdge(), QLine(  10, 2, 120, 2 ) ) );
    assertTrue(  intersects( rect2.topEdge(), QLine( 120, 2, 160, 2 ) ) );
    assertTrue(  intersects( rect2.topEdge(), QLine( 120, 2, 130, 2 ) ) );
    
    // to the left edge
    assertFalse( intersects( rect2.leftEdge(), QLine( 110, -10, 110,  0 ) ) );
    assertFalse( intersects( rect2.leftEdge(), QLine( 110,  40, 110, 50 ) ) );
    assertTrue(  intersects( rect2.leftEdge(), QLine( 110,  30, 110, 50 ) ) );
    assertTrue(  intersects( rect2.leftEdge(), QLine( 110, -10, 110, 20 ) ) );
    assertTrue(  intersects( rect2.leftEdge(), QLine( 110, -10, 110, 50 ) ) );
    assertTrue(  intersects( rect2.leftEdge(), QLine( 110,  10, 110, 20 ) ) );
    
    // customer test case - already coveredy by the coincident lines, actually
    assertFalse( rect2.intersects( QLine( 92, 37, 21, 37 ) ) );

#ifdef KDAB_HAVE_CPP11_CONSTEXPR
    {
        // check that KDRect can be used as a generalised constant expression:
#define CHECK_IS_CONSTEXPR( which ) \
        switch ( 12 ) { case which: ; default: ; }

        // check that properties can be used as constant expressions:
        CHECK_IS_CONSTEXPR( KDRect::fromPoints( 12, 12, 24, 24 ).left() );
        CHECK_IS_CONSTEXPR( KDRect::fromTopLeftAndSize( 12, 12, 200, 400 ).right() );
        CHECK_IS_CONSTEXPR( KDRect::fromBottomRightAndSize( 12, 12, 200, 400 ).top() );
        CHECK_IS_CONSTEXPR( KDRect::fromTopRightAndSize( 12, 12, 200, 400 ).bottom() );
        CHECK_IS_CONSTEXPR( KDRect::fromBottomLeftAndSize( 12, 12, 200, 400 ).hCenter() );
        CHECK_IS_CONSTEXPR( KDRect::fromCenterPointAndSize( 12, 12, 200, 400 ). vCenter() );
        CHECK_IS_CONSTEXPR( KDRect::fromPoints( KDPoint( 12, 12 ), 24, 24 ).left() );
        CHECK_IS_CONSTEXPR( KDRect::fromPoints( 12, 12 , KDPoint( 24, 24 ) ).left() );
        CHECK_IS_CONSTEXPR( KDRect::fromPoints( KDPoint( 12, 12 ), KDPoint( 24, 24 ) ).left() );
        CHECK_IS_CONSTEXPR( KDRect::fromTopLeftAndSize( KDPoint( 12, 12 ), 200, 400 ).right() );
        CHECK_IS_CONSTEXPR( KDRect::fromBottomRightAndSize( KDPoint( 12, 12 ), 200, 400 ).top() );
        CHECK_IS_CONSTEXPR( KDRect::fromTopRightAndSize( KDPoint( 12, 12 ), 200, 400 ).bottom() );
        CHECK_IS_CONSTEXPR( KDRect::fromBottomLeftAndSize( KDPoint( 12, 12 ), 200, 400 ).hCenter() );
        CHECK_IS_CONSTEXPR( KDRect::fromCenterPointAndSize( KDPoint( 12, 12 ), 200, 400 ). vCenter() );
        CHECK_IS_CONSTEXPR( KDRect().normalized().width() );
        CHECK_IS_CONSTEXPR( KDRect().height() );
        CHECK_IS_CONSTEXPR( KDRect().area() );
        CHECK_IS_CONSTEXPR( KDRect().isEmpty() );
        CHECK_IS_CONSTEXPR( KDRect().isNull() );
        CHECK_IS_CONSTEXPR( KDRect().isValid() );
        CHECK_IS_CONSTEXPR( KDRect().contains( 0, 0 ) );
        CHECK_IS_CONSTEXPR( KDRect().contains( KDPoint( 0, 0 ) ) );
        CHECK_IS_CONSTEXPR( KDRect().contains( KDRect::fromCenterPointAndSize( 0, 0, 2, 2 ) ) );
        CHECK_IS_CONSTEXPR( KDRect().intersected( KDRect() ).left() );
        CHECK_IS_CONSTEXPR( KDRect().united( KDRect() ).left() );
        CHECK_IS_CONSTEXPR( KDRect().united( 12, 12 ).left() );
        CHECK_IS_CONSTEXPR( KDRect().movedBy( 12, 12 ).left() );
        CHECK_IS_CONSTEXPR( KDRect().translated( 12, 12 ).left() );
        CHECK_IS_CONSTEXPR( KDRect().movedTo( 12, 12 ).left() );
        CHECK_IS_CONSTEXPR( KDRect().united( KDPoint( 12, 12 ) ).left() );
        CHECK_IS_CONSTEXPR( KDRect().movedBy( KDPoint( 12, 12 ) ).left() );
        CHECK_IS_CONSTEXPR( KDRect().translated( KDPoint( 12, 12 ) ).left() );
        CHECK_IS_CONSTEXPR( KDRect().movedTo( KDPoint( 12, 12 ) ).left() );
        CHECK_IS_CONSTEXPR( KDRect().topLeftMoved( 12, 12 ).left() );
        CHECK_IS_CONSTEXPR( KDRect().topLeftMoved( KDPoint( 12, 12 ) ).left() );
        CHECK_IS_CONSTEXPR( KDRect().topRightMoved( 12, 12 ).left() );
        CHECK_IS_CONSTEXPR( KDRect().topRightMoved( KDPoint( 12, 12 ) ).left() );
        CHECK_IS_CONSTEXPR( KDRect().topCenterMoved( 12, 12 ).left() );
        CHECK_IS_CONSTEXPR( KDRect().topCenterMoved( KDPoint( 12, 12 ) ).left() );
        CHECK_IS_CONSTEXPR( KDRect().rightCenterMoved( 12, 12 ).left() );
        CHECK_IS_CONSTEXPR( KDRect().rightCenterMoved( KDPoint( 12, 12 ) ).left() );
        CHECK_IS_CONSTEXPR( KDRect().centerMoved( 12, 12 ).left() );
        CHECK_IS_CONSTEXPR( KDRect().centerMoved( KDPoint( 12, 12 ) ).left() );
        CHECK_IS_CONSTEXPR( KDRect().leftCenterMoved( 12, 12 ).left() );
        CHECK_IS_CONSTEXPR( KDRect().leftCenterMoved( KDPoint( 12, 12 ) ).left() );
        CHECK_IS_CONSTEXPR( KDRect().bottomLeftMoved( 12, 12 ).left() );
        CHECK_IS_CONSTEXPR( KDRect().bottomLeftMoved( KDPoint( 12, 12 ) ).left() );
        CHECK_IS_CONSTEXPR( KDRect().bottomRightMoved( 12, 12 ).left() );
        CHECK_IS_CONSTEXPR( KDRect().bottomRightMoved( KDPoint( 12, 12 ) ).left() );
        CHECK_IS_CONSTEXPR( KDRect().bottomCenterMoved( 12, 12 ).left() );
        CHECK_IS_CONSTEXPR( KDRect().bottomCenterMoved( KDPoint( 12, 12 ) ).left() );
        CHECK_IS_CONSTEXPR( KDRect().topMoved( 12 ).left() );
        CHECK_IS_CONSTEXPR( KDRect().leftMoved( 12 ).left() );
        CHECK_IS_CONSTEXPR( KDRect().rightMoved( 12 ).left() );
        CHECK_IS_CONSTEXPR( KDRect().bottomMoved( 12 ).left() );
        CHECK_IS_CONSTEXPR( KDRect().vCenterMoved( 12 ).left() );
        CHECK_IS_CONSTEXPR( KDRect().hCenterMoved( 12 ).left() );
        CHECK_IS_CONSTEXPR( KDRect().resized( 12, 12 ).left() );
        CHECK_IS_CONSTEXPR( KDRect().transposed().left() );
        CHECK_IS_CONSTEXPR( KDRect().mirrored( 12, 12 ).left() );
        CHECK_IS_CONSTEXPR( KDRect().mirrored( KDPoint( 12, 12 ) ).left() );
        CHECK_IS_CONSTEXPR( KDRect().mirrored().left() );
        CHECK_IS_CONSTEXPR( KDRect().adjusted( -1, -1, 1, 1 ).left() );
        CHECK_IS_CONSTEXPR( KDRect().grown( 12 ).shrunken( 11 ).left() );
        CHECK_IS_CONSTEXPR( KDRect().topLeft().x() );
        CHECK_IS_CONSTEXPR( KDRect().topCenter().x() );
        CHECK_IS_CONSTEXPR( KDRect().topRight().x() );
        CHECK_IS_CONSTEXPR( KDRect().leftCenter().x() );
        CHECK_IS_CONSTEXPR( KDRect().center().x() );
        CHECK_IS_CONSTEXPR( KDRect().rightCenter().x() );
        CHECK_IS_CONSTEXPR( KDRect().bottomLeft().y() );
        CHECK_IS_CONSTEXPR( KDRect().bottomCenter().y() );
        CHECK_IS_CONSTEXPR( KDRect().bottomRight().y() );
#if QT_VERSION >= 0x040800
        CHECK_IS_CONSTEXPR( KDRect().referencePoint( Qt::AlignCenter ).x() );
        CHECK_IS_CONSTEXPR( KDRect().referencePointMoved( Qt::AlignCenter, KDPoint( 12, 12 ) ).left() );
        CHECK_IS_CONSTEXPR( KDRect().aligned( KDRect(), Qt::AlignCenter ).left() );
#endif
        CHECK_IS_CONSTEXPR( KDRect().referencePoint( Qt::TopLeftCorner ).x() );
        CHECK_IS_CONSTEXPR( KDRect().referencePointMoved( Qt::TopLeftCorner, KDPoint( 12, 12 ) ).left() );
        CHECK_IS_CONSTEXPR( KDRect().aligned( KDRect(), Qt::TopLeftCorner ).left() );
        CHECK_IS_CONSTEXPR( ( KDRect() & KDRect() ).left() );
        CHECK_IS_CONSTEXPR( ( KDRect() | KDRect() ).left() );
        CHECK_IS_CONSTEXPR( ( KDRect() | KDPoint() ).left() );
        CHECK_IS_CONSTEXPR( ( KDPoint() | KDRect() ).left() );
        CHECK_IS_CONSTEXPR( ( KDPoint() + KDPoint() ).x() );
        CHECK_IS_CONSTEXPR( ( KDPoint() - KDPoint() ).y() );

#undef CHECK_IS_CONSTEXPR
    }
#endif

    {
        static KDAB_DECL_CONSTEXPR_VAR KDPoint points[] = {
            KDPoint( 1, 1 ),
            KDPoint( 2, 2 ),
            KDPoint( 3, 3 ),
        };
        static KDAB_DECL_CONSTEXPR_VAR  KDRect rects[] = {
            KDRect::fromPoints( 0, 0, 12, 12 ),
            KDRect::fromPoints( 12, 12, 24, 24 ),
        };

#if 0 // check compilation
        QPainter p;
        p.drawRect( rects[0] );
        p.drawRects( &static_cast<const QRect&>(rects[0]), sizeof rects / sizeof *rects );
        p.drawPoint( points[0] );
        p.drawPoints( &static_cast<const QPoint&>(points[0]), sizeof points / sizeof *points );
#endif

        (void)rects;
        (void)points;
    }

    {
        const KDPoint p1( 1, 2 );
        const QPoint  p2( 1, 2 );
        const KDPoint p3 = p2;
        assertEqual( p1, p2 ); // check the funny OSX member variable layout
        assertEqual( p2, p3 );
    }

}

#endif // KDTOOLSCORE_UNITTESTS
