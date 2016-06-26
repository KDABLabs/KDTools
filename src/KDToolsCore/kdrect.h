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

#ifndef __KDTOOLSCORE_KDRECT_H__
#define __KDTOOLSCORE_KDRECT_H__

#include <KDToolsCore/kdtoolsglobal.h>

#include <QtCore/QRect>
#include <QtCore/QLine>
#include <QtCore/QPoint>
#include <QtCore/QSize>

#if QT_VERSION >= 0x040800
# define KDRECT_CONSTEXPR_FOR_QFLAGS KDAB_DECL_CONSTEXPR
#else
# define KDRECT_CONSTEXPR_FOR_QFLAGS
#endif

class KDTOOLSCORE_EXPORT KDPoint KDAB_FINAL_CLASS {
public:
    KDAB_DECL_CONSTEXPR KDPoint();
    KDAB_DECL_CONSTEXPR KDPoint( int x, int y );

    KDPoint( const QPoint & p );
    KDPoint & operator=( const QPoint & p ) { return operator=( reinterpret_cast<const KDPoint&>( p ) ); }
    operator const QPoint & () const { return reinterpret_cast<const QPoint&>( *this ); }

    KDAB_DECL_CONSTEXPR int x() const { return xp; }
    KDAB_DECL_CONSTEXPR int y() const { return yp; }

private:
#if defined(Q_OS_MAC) && !defined(QT_NO_CORESERVICES)
    int yp;
    int xp;
#else
    int xp;
    int yp;
#endif
};

#if defined(Q_OS_MAC) && !defined(QT_NO_CORESERVICES)
KDAB_DECL_CONSTEXPR KDPoint::KDPoint() : yp( 0 ), xp( 0 ) {}
KDAB_DECL_CONSTEXPR KDPoint::KDPoint( int x, int y ) : yp( y ), xp( x ) {}
KDPoint::KDPoint( const QPoint & p ) : yp( p.y() ), xp( p.x() ) {}
#else
KDAB_DECL_CONSTEXPR KDPoint::KDPoint() : xp( 0 ), yp( 0 ) {}
KDAB_DECL_CONSTEXPR KDPoint::KDPoint( int x, int y ) : xp( x ), yp( y ) {}
KDPoint::KDPoint( const QPoint & p ) : xp( p.x() ), yp( p.y() ) {}
#endif

inline KDAB_DECL_CONSTEXPR KDPoint operator+( const KDPoint & lhs, const KDPoint & rhs ) {
    return KDPoint( lhs.x() + rhs.x(), lhs.y() + rhs.y() );
}
inline KDPoint operator+( const QPoint & lhs, const KDPoint & rhs ) {
    return KDPoint( lhs.x() + rhs.x(), lhs.y() + rhs.y() );
}
inline KDPoint operator+( const KDPoint & lhs, const QPoint & rhs ) {
    return KDPoint( lhs.x() + rhs.x(), lhs.y() + rhs.y() );
}

inline KDAB_DECL_CONSTEXPR KDPoint operator-( const KDPoint & lhs, const KDPoint & rhs ) {
    return KDPoint( lhs.x() - rhs.x(), lhs.y() - rhs.y() );
}
inline KDPoint operator-( const QPoint & lhs, const KDPoint & rhs ) {
    return KDPoint( lhs.x() - rhs.x(), lhs.y() - rhs.y() );
}
inline KDPoint operator-( const KDPoint & lhs, const QPoint & rhs ) {
    return KDPoint( lhs.x() - rhs.x(), lhs.y() - rhs.y() );
}

class KDTOOLSCORE_EXPORT KDRect KDAB_FINAL_CLASS {
    KDAB_DECL_CONSTEXPR KDRect( int x1, int y1, int x2, int y2 );
public:
    KDAB_DECL_CONSTEXPR KDRect();
    KDRect( const QRect & rect );

    KDRect & operator=( const QRect & rect );

    operator const QRect & () const;

    static KDRect fromRect( const QRect & rect );

    static KDAB_DECL_CONSTEXPR KDRect fromPoints( int l, int t, int r, int b );
    static KDRect fromPoints( const QPoint & tl, int r, int b );
    static KDAB_DECL_CONSTEXPR KDRect fromPoints( const KDPoint & tl, int r, int b );
    static KDRect fromPoints( int l, int t, const QPoint & br );
    static KDAB_DECL_CONSTEXPR KDRect fromPoints( int l, int t, const KDPoint & br );
    static KDRect fromPoints( const QPoint & tl, const QPoint & br );
    static KDAB_DECL_CONSTEXPR KDRect fromPoints( const KDPoint & tl, const KDPoint & br );

    static KDAB_DECL_CONSTEXPR KDRect fromTopLeftAndSize( int l, int t, int w, int h );
    static KDRect fromTopLeftAndSize( int l, int t, const QSize & sz );
    static KDRect fromTopLeftAndSize( const QPoint & tl, int w, int h );
    static KDAB_DECL_CONSTEXPR KDRect fromTopLeftAndSize( const KDPoint & tl, int w, int h );
    static KDRect fromTopLeftAndSize( const QPoint & tl, const QSize & sz );

    static KDAB_DECL_CONSTEXPR KDRect fromTopRightAndSize( int r, int t, int w, int h );
    static KDRect fromTopRightAndSize( int r, int t, const QSize & sz );
    static KDRect fromTopRightAndSize( const QPoint & tr, int w, int h );
    static KDAB_DECL_CONSTEXPR KDRect fromTopRightAndSize( const KDPoint & tr, int w, int h );
    static KDRect fromTopRightAndSize( const QPoint & tr, const QSize & sz );

    static KDAB_DECL_CONSTEXPR KDRect fromBottomLeftAndSize( int l, int b, int w, int h );
    static KDRect fromBottomLeftAndSize( int l, int b, const QSize & sz );
    static KDRect fromBottomLeftAndSize( const QPoint & bl, int w, int h );
    static KDAB_DECL_CONSTEXPR KDRect fromBottomLeftAndSize( const KDPoint & bl, int w, int h );
    static KDRect fromBottomLeftAndSize( const QPoint & bl, const QSize & sz );

    static KDAB_DECL_CONSTEXPR KDRect fromBottomRightAndSize( int l, int b, int w, int h );
    static KDRect fromBottomRightAndSize( int l, int b, const QSize & sz );
    static KDRect fromBottomRightAndSize( const QPoint & br, int w, int h );
    static KDAB_DECL_CONSTEXPR KDRect fromBottomRightAndSize( const KDPoint & br, int w, int h );
    static KDRect fromBottomRightAndSize( const QPoint & br, const QSize & sz );

    static KDAB_DECL_CONSTEXPR KDRect fromCenterPointAndSize( int x, int y, int w, int h );
    static KDRect fromCenterPointAndSize( int x, int y, const QSize & sz );
    static KDRect fromCenterPointAndSize( const QPoint & c, int w, int h );
    static KDAB_DECL_CONSTEXPR KDRect fromCenterPointAndSize( const KDPoint & c, int w, int h );
    static KDRect fromCenterPointAndSize( const QPoint & c, const QSize & sz );


    KDAB_DECL_CONSTEXPR bool isNull() const;
    KDAB_DECL_CONSTEXPR bool isValid() const;
    KDAB_DECL_CONSTEXPR bool isEmpty() const;


    KDAB_DECL_CONSTEXPR bool contains( int x, int y ) const;
    bool contains( const QPoint & p ) const;
    KDAB_DECL_CONSTEXPR bool contains( const KDPoint & p ) const;

    bool contains( const QRect & rect ) const;
    KDAB_DECL_CONSTEXPR bool contains( const KDRect & rect ) const;

    bool intersects( const QRect & rect ) const;
    bool intersects( const KDRect & rect ) const;

    bool intersects( const QLine & line ) const;


    KDAB_DECL_CONSTEXPR int left() const;
    KDAB_DECL_CONSTEXPR int top() const;
    KDAB_DECL_CONSTEXPR int right() const;
    KDAB_DECL_CONSTEXPR int bottom() const;
    KDAB_DECL_CONSTEXPR int hCenter() const;
    KDAB_DECL_CONSTEXPR int vCenter() const;

    KDAB_DECL_CONSTEXPR int area() const;

    KDAB_DECL_CONSTEXPR int height() const;
    KDAB_DECL_CONSTEXPR int width() const;
    QSize size() const;

    KDAB_DECL_CONSTEXPR KDPoint topLeft() const;
    KDAB_DECL_CONSTEXPR KDPoint topCenter() const;
    KDAB_DECL_CONSTEXPR KDPoint topRight() const;

    KDAB_DECL_CONSTEXPR KDPoint leftCenter() const;
    KDAB_DECL_CONSTEXPR KDPoint center() const;
    KDAB_DECL_CONSTEXPR KDPoint rightCenter() const;

    KDAB_DECL_CONSTEXPR KDPoint bottomLeft() const;
    KDAB_DECL_CONSTEXPR KDPoint bottomCenter() const;
    KDAB_DECL_CONSTEXPR KDPoint bottomRight() const;

    KDRECT_CONSTEXPR_FOR_QFLAGS KDPoint referencePoint( Qt::Alignment align ) const;
    KDAB_DECL_CONSTEXPR KDPoint referencePoint( Qt::Corner corner ) const;

    QLine topEdge() const;
    QLine leftEdge() const;
    QLine rightEdge() const;
    QLine bottomEdge() const;

    QLine hCenterLine() const;
    QLine vCenterLine() const;

    QLine topLeftDiagonal() const;
    QLine bottomLeftDiagonal() const;


    KDAB_DECL_CONSTEXPR KDRect normalized() const;

    KDAB_DECL_CONSTEXPR KDRect intersected( const KDRect & rect ) const;
    KDRect intersected( const QRect & rect ) const;

    KDAB_DECL_CONSTEXPR KDRect united( const KDRect & rect ) const;
    KDRect united( const QRect & rect ) const;

    KDAB_DECL_CONSTEXPR KDRect united( int x, int y ) const;
    KDRect united( const QPoint & p ) const;
    KDAB_DECL_CONSTEXPR KDRect united( const KDPoint & p ) const;

    KDAB_DECL_CONSTEXPR KDRect movedBy( int dx, int dy ) const;
    KDRect movedBy( const QPoint & delta ) const;
    KDAB_DECL_CONSTEXPR KDRect movedBy( const KDPoint & delta ) const;

    KDAB_DECL_CONSTEXPR KDRect translated( int dx, int dy ) const;
    KDRect translated( const QPoint & delta ) const;
    KDAB_DECL_CONSTEXPR KDRect translated( const KDPoint & delta ) const;

    KDAB_DECL_CONSTEXPR KDRect movedTo( int l, int t ) const;
    KDRect movedTo( const QPoint & tl ) const;
    KDAB_DECL_CONSTEXPR KDRect movedTo( const KDPoint & tl ) const;

    KDAB_DECL_CONSTEXPR KDRect topMoved( int t ) const;
    KDAB_DECL_CONSTEXPR KDRect leftMoved( int l ) const;
    KDAB_DECL_CONSTEXPR KDRect rightMoved( int r ) const;
    KDAB_DECL_CONSTEXPR KDRect bottomMoved( int b ) const;
    KDAB_DECL_CONSTEXPR KDRect vCenterMoved( int v ) const;
    KDAB_DECL_CONSTEXPR KDRect hCenterMoved( int h ) const;

    KDRect referencePointMoved( Qt::Alignment align, const QPoint & p ) const;
    KDRECT_CONSTEXPR_FOR_QFLAGS KDRect referencePointMoved( Qt::Alignment align, const KDPoint & p ) const;
    KDRECT_CONSTEXPR_FOR_QFLAGS KDRect referencePointMoved( Qt::Alignment align, int x, int y ) const;
    KDRect referencePointMoved( Qt::Corner corner, const QPoint & p ) const;
    KDAB_DECL_CONSTEXPR KDRect referencePointMoved( Qt::Corner corner, const KDPoint & p ) const;
    KDAB_DECL_CONSTEXPR KDRect referencePointMoved( Qt::Corner corner, int x, int y ) const;

    KDAB_DECL_CONSTEXPR KDRect topLeftMoved( int l, int t ) const;
    KDRect topLeftMoved( const QPoint & tl ) const;
    KDAB_DECL_CONSTEXPR KDRect topLeftMoved( const KDPoint & tl ) const;
    KDAB_DECL_CONSTEXPR KDRect topCenterMoved( int c, int t ) const;
    KDRect topCenterMoved( const QPoint & tc ) const;
    KDAB_DECL_CONSTEXPR KDRect topCenterMoved( const KDPoint & tc ) const;
    KDAB_DECL_CONSTEXPR KDRect topRightMoved( int r, int t ) const;
    KDRect topRightMoved( const QPoint & tr ) const;
    KDAB_DECL_CONSTEXPR KDRect topRightMoved( const KDPoint & tr ) const;
    KDAB_DECL_CONSTEXPR KDRect leftCenterMoved( int l, int c ) const;
    KDRect leftCenterMoved( const QPoint & lc ) const;
    KDAB_DECL_CONSTEXPR KDRect leftCenterMoved( const KDPoint & lc ) const;
    KDAB_DECL_CONSTEXPR KDRect centerMoved( int x, int y ) const;
    KDRect centerMoved( const QPoint & c ) const;
    KDAB_DECL_CONSTEXPR KDRect centerMoved( const KDPoint & c ) const;
    KDAB_DECL_CONSTEXPR KDRect rightCenterMoved( int r, int c ) const;
    KDRect rightCenterMoved( const QPoint & rc ) const;
    KDAB_DECL_CONSTEXPR KDRect rightCenterMoved( const KDPoint & rc ) const;
    KDAB_DECL_CONSTEXPR KDRect bottomLeftMoved( int l, int b ) const;
    KDRect bottomLeftMoved( const QPoint & bl ) const;
    KDAB_DECL_CONSTEXPR KDRect bottomLeftMoved( const KDPoint & bl ) const;
    KDAB_DECL_CONSTEXPR KDRect bottomCenterMoved( int c, int b ) const;
    KDRect bottomCenterMoved( const QPoint & bc ) const;
    KDAB_DECL_CONSTEXPR KDRect bottomCenterMoved( const KDPoint & bc ) const;
    KDAB_DECL_CONSTEXPR KDRect bottomRightMoved( int r, int b ) const;
    KDRect bottomRightMoved( const QPoint & br ) const;
    KDAB_DECL_CONSTEXPR KDRect bottomRightMoved( const KDPoint & br ) const;

    KDRECT_CONSTEXPR_FOR_QFLAGS KDRect aligned( const KDRect & ref, Qt::Alignment align ) const;
    KDAB_DECL_CONSTEXPR KDRect aligned( const KDRect & ref, Qt::Corner corner ) const;
    KDRect aligned( const QRect & ref, Qt::Alignment align ) const;
    KDRect aligned( const QRect & ref, Qt::Corner corner ) const;

    KDAB_DECL_CONSTEXPR KDRect resized( int w, int h ) const;
    KDRect resized( const QSize & sz ) const;

    KDAB_DECL_CONSTEXPR KDRect transposed() const;

    KDAB_DECL_CONSTEXPR KDRect mirrored( int x, int y ) const;
    KDRect mirrored( const QPoint & p ) const;
    KDAB_DECL_CONSTEXPR KDRect mirrored( const KDPoint & p ) const;
    KDAB_DECL_CONSTEXPR KDRect mirrored() const;

    KDAB_DECL_CONSTEXPR KDRect adjusted( int dl, int dt, int dr, int db ) const;

    KDAB_DECL_CONSTEXPR KDRect grown( int delta ) const;
    KDAB_DECL_CONSTEXPR KDRect shrunken( int delta ) const;

    KDRect expandedTo( int w, int h ) const;
    KDRect expandedTo( const QSize & sz ) const;

    KDRect boundedTo( int w, int h ) const;
    KDRect boundedTo( const QSize & sz ) const;

private:
    KDAB_DECL_CONSTEXPR KDRect referencePointMovedImpl( int align, const KDPoint & p ) const;
    KDAB_DECL_CONSTEXPR KDPoint referencePointImpl( int align ) const;


private:
#ifdef Q_OS_MAC
    int y1;
    int x1;
    int y2;
    int x2;
#else
    int x1;
    int y1;
    int x2;
    int y2;
#endif
};

inline KDAB_DECL_CONSTEXPR bool operator==( const KDRect & lhs, const KDRect & rhs ) {
    return lhs.top() == rhs.top() && lhs.left() == rhs.left() && lhs.right() == rhs.right() && lhs.bottom() == rhs.bottom() ;
}
inline KDAB_DECL_CONSTEXPR bool operator!=( const KDRect & lhs, const KDRect & rhs ) {
    return !operator==( lhs, rhs );
}

// need the mixed ones for dismbiguation
inline bool operator==( const QRect & lhs, const KDRect & rhs ) {
    return lhs.top() == rhs.top() && lhs.left() == rhs.left() && lhs.right() == rhs.right() && lhs.bottom() == rhs.bottom() ;
}
inline bool operator!=( const QRect & lhs, const KDRect & rhs ) {
    return !operator==( lhs, rhs );
}

inline bool operator==( const KDRect & lhs, const QRect & rhs ) {
    return lhs.top() == rhs.top() && lhs.left() == rhs.left() && lhs.right() == rhs.right() && lhs.bottom() == rhs.bottom() ;
}
inline bool operator!=( const KDRect & lhs, const QRect & rhs ) {
    return !operator==( lhs, rhs );
}

inline KDAB_DECL_CONSTEXPR KDRect operator&( const KDRect & lhs, const KDRect & rhs ) {
    return lhs.intersected( rhs );
}
inline KDRect operator&( const QRect & lhs, const KDRect & rhs ) {
    return rhs.intersected( lhs );
}
inline KDRect operator&( const KDRect & lhs, const QRect & rhs ) {
    return lhs.intersected( rhs );
}

inline KDAB_DECL_CONSTEXPR KDRect operator|( const KDRect & lhs, const KDRect & rhs ) {
    return lhs.united( rhs );
}
inline KDRect operator|( const QRect & lhs, const KDRect & rhs ) {
    return rhs.united( lhs );
}
inline KDRect operator|( const KDRect & lhs, const QRect & rhs ) {
    return lhs.united( rhs );
}

inline KDRect operator|( const KDRect & lhs, const QPoint & rhs ) {
    return lhs.united( rhs );
}
inline KDAB_DECL_CONSTEXPR KDRect operator|( const KDRect & lhs, const KDPoint & rhs ) {
    return lhs.united( rhs );
}
inline KDRect operator|( const QPoint & lhs, const KDRect & rhs ) {
    return rhs.united( lhs );
}
inline KDAB_DECL_CONSTEXPR KDRect operator|( const KDPoint & lhs, const KDRect & rhs ) {
    return rhs.united( lhs );
}
inline KDRect operator|( const QRect & lhs, const QPoint & rhs ) {
    return reinterpret_cast<const KDRect&>( lhs ).united( rhs );
}
inline KDRect operator|( const QRect & lhs, const KDPoint & rhs ) {
    return reinterpret_cast<const KDRect&>( lhs ).united( rhs );
}
inline KDRect operator|( const QPoint & lhs, const QRect & rhs ) {
    return reinterpret_cast<const KDRect&>( rhs ).united( lhs );
}
inline KDRect operator|( const KDPoint & lhs, const QRect & rhs ) {
    return reinterpret_cast<const KDRect&>( rhs ).united( lhs );
}

//
// implementation
//

#ifdef Q_OS_MAC
inline KDAB_DECL_CONSTEXPR KDRect::KDRect() : y1( 0 ), x1( 0 ), y2( -1 ), x2( -1 ) {}
inline KDAB_DECL_CONSTEXPR KDRect::KDRect( int x1_, int y1_, int x2_, int y2_ ) : y1( y1_ ), x1( x1_ ), y2( y2_ ), x2( x2_ ) {}
inline KDRect::KDRect( const QRect & r ) : y1( r.top() ), x1( r.left() ), y2( r.bottom() ), x2( r.right() ) {}
#else
inline KDAB_DECL_CONSTEXPR KDRect::KDRect() : x1( 0 ), y1( 0 ), x2( -1 ), y2( -1 ) {}
inline KDAB_DECL_CONSTEXPR KDRect::KDRect( int x1_, int y1_, int x2_, int y2_ ) : x1( x1_ ), y1( y1_ ), x2( x2_ ), y2( y2_ ) {}
inline KDRect::KDRect( const QRect & r ) : x1( r.left() ), y1( r.top() ), x2( r.right() ), y2( r.bottom() ) {}
#endif

inline KDRect & KDRect::operator=( const QRect & rect ) {
    return operator=( reinterpret_cast<const KDRect&>( rect ) );
}

inline KDRect::operator const QRect & () const {
    return reinterpret_cast<const QRect &>( *this );
}

inline KDRect KDRect::fromRect( const QRect & r ) { return r; }

inline KDAB_DECL_CONSTEXPR KDRect KDRect::fromPoints( int l, int t, int r, int b ) { return KDRect( l, t, r, b ); }
inline KDRect KDRect::fromPoints( const QPoint & tl, int r, int b ) { return KDRect( tl.x(), tl.y(), r, b ); }
inline KDAB_DECL_CONSTEXPR KDRect KDRect::fromPoints( const KDPoint & tl, int r, int b ) { return KDRect( tl.x(), tl.y(), r, b ); }
inline KDRect KDRect::fromPoints( int l, int t, const QPoint & br ) { return KDRect( l, t, br.x(), br.y() ); }
inline KDAB_DECL_CONSTEXPR KDRect KDRect::fromPoints( int l, int t, const KDPoint & br ) { return KDRect( l, t, br.x(), br.y() ); }
inline KDRect KDRect::fromPoints( const QPoint & tl, const QPoint & br ) { return KDRect( tl.x(), tl.y(), br.x(), br.y() ); }
inline KDAB_DECL_CONSTEXPR KDRect KDRect::fromPoints( const KDPoint & tl, const KDPoint & br ) { return KDRect( tl.x(), tl.y(), br.x(), br.y() ); }

inline KDAB_DECL_CONSTEXPR KDRect KDRect::fromTopLeftAndSize( int l, int t, int w, int h ) { return KDRect( l, t, l+w-1, t+h-1 ); }
inline KDRect KDRect::fromTopLeftAndSize( int l, int t, const QSize & sz ) { return fromTopLeftAndSize( l, t, sz.width(), sz.height() ); }
inline KDRect KDRect::fromTopLeftAndSize( const QPoint & tl, int w, int h ) { return fromTopLeftAndSize( tl.x(), tl.y(), w, h ); }
inline KDAB_DECL_CONSTEXPR KDRect KDRect::fromTopLeftAndSize( const KDPoint & tl, int w, int h ) { return fromTopLeftAndSize( tl.x(), tl.y(), w, h ); }
inline KDRect KDRect::fromTopLeftAndSize( const QPoint & tl, const QSize & sz ) { return fromTopLeftAndSize( tl.x(), tl.y(), sz.width(), sz.height() ); }

inline KDAB_DECL_CONSTEXPR KDRect KDRect::fromTopRightAndSize( int r, int t, int w, int h ) { return KDRect( r-w+1, t, r, t+h-1 ); }
inline KDRect KDRect::fromTopRightAndSize( int r, int t, const QSize & sz ) { return fromTopRightAndSize( r, t, sz.width(), sz.height() ); }
inline KDRect KDRect::fromTopRightAndSize( const QPoint & tr, int w, int h ) { return fromTopRightAndSize( tr.x(), tr.y(), w, h ); }
inline KDAB_DECL_CONSTEXPR KDRect KDRect::fromTopRightAndSize( const KDPoint & tr, int w, int h ) { return fromTopRightAndSize( tr.x(), tr.y(), w, h ); }
inline KDRect KDRect::fromTopRightAndSize( const QPoint & tr, const QSize & sz ) { return fromTopRightAndSize( tr.x(), tr.y(), sz.width(), sz.height() ); }

inline KDAB_DECL_CONSTEXPR KDRect KDRect::fromBottomLeftAndSize( int l, int b, int w, int h ) { return KDRect( l, b-h+1, l+w-1, b ); }
inline KDRect KDRect::fromBottomLeftAndSize( int l, int b, const QSize & sz ) { return fromBottomLeftAndSize( l, b, sz.width(), sz.height() ); }
inline KDRect KDRect::fromBottomLeftAndSize( const QPoint & bl, int w, int h ) { return fromBottomLeftAndSize( bl.x(), bl.y(), w, h ); }
inline KDAB_DECL_CONSTEXPR KDRect KDRect::fromBottomLeftAndSize( const KDPoint & bl, int w, int h ) { return fromBottomLeftAndSize( bl.x(), bl.y(), w, h ); }
inline KDRect KDRect::fromBottomLeftAndSize( const QPoint & bl, const QSize & sz ) { return fromBottomLeftAndSize( bl.x(), bl.y(), sz.width(), sz.height() ); }

inline KDAB_DECL_CONSTEXPR KDRect KDRect::fromBottomRightAndSize( int r, int b, int w, int h ) { return KDRect( r-w+1, b-h+1, r, b ); }
inline KDRect KDRect::fromBottomRightAndSize( int r, int b, const QSize & sz ) { return fromBottomRightAndSize( r, b, sz.width(), sz.height() ); }
inline KDRect KDRect::fromBottomRightAndSize( const QPoint & br, int w, int h ) { return fromBottomRightAndSize( br.x(), br.y(), w, h ); }
inline KDAB_DECL_CONSTEXPR KDRect KDRect::fromBottomRightAndSize( const KDPoint & br, int w, int h ) { return fromBottomRightAndSize( br.x(), br.y(), w, h ); }
inline KDRect KDRect::fromBottomRightAndSize( const QPoint & br, const QSize & sz ) { return fromBottomRightAndSize( br.x(), br.y(), sz.width(), sz.height() ); }

inline KDAB_DECL_CONSTEXPR KDRect KDRect::fromCenterPointAndSize( int x, int y, int w, int h ) { return fromTopLeftAndSize( x-w/2, y-h/h, w, h ); }
inline KDRect KDRect::fromCenterPointAndSize( int x, int y, const QSize & sz ) { return fromCenterPointAndSize( x, y, sz.width(), sz.height() ); }
inline KDRect KDRect::fromCenterPointAndSize( const QPoint & c, int w, int h ) { return fromCenterPointAndSize( c.x(), c.y(), w, h ); }
inline KDAB_DECL_CONSTEXPR KDRect KDRect::fromCenterPointAndSize( const KDPoint & c, int w, int h ) { return fromCenterPointAndSize( c.x(), c.y(), w, h ); }
inline KDRect KDRect::fromCenterPointAndSize( const QPoint & c, const QSize & sz ) { return fromCenterPointAndSize( c.x(), c.y(), sz.width(), sz.height() ); }

inline KDAB_DECL_CONSTEXPR bool KDRect::isNull()  const { return width() == 0 && height() == 0 ; }
inline KDAB_DECL_CONSTEXPR bool KDRect::isEmpty() const { return width() <= 0 || height() <= 0 ; }
inline KDAB_DECL_CONSTEXPR bool KDRect::isValid() const { return width() >  0 && height() >  0 ; }

inline KDAB_DECL_CONSTEXPR bool KDRect::contains( int x, int y ) const { return x1 <= x && x <= x2 && y1 <= y && y <= y2 ; }
inline bool KDRect::contains( const QPoint & p ) const { return contains( p.x(), p.y() ); }
inline KDAB_DECL_CONSTEXPR bool KDRect::contains( const KDPoint & p ) const { return contains( p.x(), p.y() ); }

inline KDAB_DECL_CONSTEXPR bool KDRect::contains( const KDRect & rect ) const { return contains( rect.left(), rect.top() ) && contains( rect.right(), rect.bottom() ) ; }
inline bool KDRect::contains( const QRect & rect ) const { return contains( reinterpret_cast<const KDRect &>( rect ) ); }

//inline bool KDRect::intersects( const KDRect & rect ) const;
inline bool KDRect::intersects( const QRect & rect ) const { return intersects( reinterpret_cast<const KDRect &>( rect ) ); }

//inline bool KDRect::intersects( const QLine & line ) const;

inline KDAB_DECL_CONSTEXPR int KDRect::left()    const { return x1; }
inline KDAB_DECL_CONSTEXPR int KDRect::right()   const { return x2; }
inline KDAB_DECL_CONSTEXPR int KDRect::top()     const { return y1; }
inline KDAB_DECL_CONSTEXPR int KDRect::bottom()  const { return y2; }
inline KDAB_DECL_CONSTEXPR int KDRect::hCenter() const { return ( x1 + x2 ) / 2 ; }
inline KDAB_DECL_CONSTEXPR int KDRect::vCenter() const { return ( y1 + y2 ) / 2 ; }

inline KDAB_DECL_CONSTEXPR int KDRect::area()    const { return width() * height() ; }

inline KDAB_DECL_CONSTEXPR int KDRect::width()   const { return x2 - x1 + 1 ; }
inline KDAB_DECL_CONSTEXPR int KDRect::height()  const { return y2 - y1 + 1 ; }
inline QSize KDRect::size()  const { return QSize( width(), height() ); }

inline KDAB_DECL_CONSTEXPR KDPoint KDRect::topLeft()      const { return KDPoint( left(),    top()     ); }
inline KDAB_DECL_CONSTEXPR KDPoint KDRect::topCenter()    const { return KDPoint( hCenter(), top()     ); }
inline KDAB_DECL_CONSTEXPR KDPoint KDRect::topRight()     const { return KDPoint( right(),   top()     ); }
inline KDAB_DECL_CONSTEXPR KDPoint KDRect::leftCenter()   const { return KDPoint( left(),    vCenter() ); }
inline KDAB_DECL_CONSTEXPR KDPoint KDRect::center()       const { return KDPoint( hCenter(), vCenter() ); }
inline KDAB_DECL_CONSTEXPR KDPoint KDRect::rightCenter()  const { return KDPoint( right(),   vCenter() ); }
inline KDAB_DECL_CONSTEXPR KDPoint KDRect::bottomLeft()   const { return KDPoint( left(),    bottom()  ); }
inline KDAB_DECL_CONSTEXPR KDPoint KDRect::bottomCenter() const { return KDPoint( hCenter(), bottom()  ); }
inline KDAB_DECL_CONSTEXPR KDPoint KDRect::bottomRight()  const { return KDPoint( right(),   bottom()  ); }

inline KDAB_DECL_CONSTEXPR KDPoint KDRect::referencePointImpl( int align ) const {
    return KDPoint( align & Qt::AlignRight   ? right() :
                    align & Qt::AlignHCenter ? hCenter() :
                    align & Qt::AlignLeft    ? left() : -1,
                    align & Qt::AlignBottom  ? bottom() :
                    align & Qt::AlignVCenter ? vCenter() :
                    align & Qt::AlignTop     ? top() : -1 );
}
inline KDRECT_CONSTEXPR_FOR_QFLAGS KDPoint KDRect::referencePoint( Qt::Alignment align ) const { return referencePointImpl( align ); }
inline KDAB_DECL_CONSTEXPR KDPoint KDRect::referencePoint( Qt::Corner corner ) const {
    return
        corner == Qt::TopLeftCorner     ? topLeft()     :
        corner == Qt::TopRightCorner    ? topRight()    :
        corner == Qt::BottomLeftCorner  ? bottomLeft()  :
        corner == Qt::BottomRightCorner ? bottomRight() :
        /* else */                        KDPoint()     ; // can't happen, just shutting up the compiler
}

inline QLine KDRect::topEdge()    const { return QLine( x1, y1, x2, y1 ); }
inline QLine KDRect::leftEdge()   const { return QLine( x1, y1, x1, y2 ); }
inline QLine KDRect::rightEdge()  const { return QLine( x2, y1, x2, y2 ); }
inline QLine KDRect::bottomEdge() const { return QLine( x1, y2, x2, y2 ); }

inline QLine KDRect::hCenterLine() const { return QLine( x1, vCenter(), x2, vCenter() ); }
inline QLine KDRect::vCenterLine() const { return QLine( hCenter(), y1, hCenter(), y2 ); }

inline QLine KDRect::topLeftDiagonal()    const { return QLine( x1, y1, x2, y2 ); }
inline QLine KDRect::bottomLeftDiagonal() const { return QLine( x1, y2, x2, y1 ); }


inline KDAB_DECL_CONSTEXPR KDRect KDRect::normalized() const { return KDRect( kdMin( x1, x2 ), kdMin( y1, y2 ), kdMax( x1, x2 ), kdMax( y1, y2 ) ); }

inline KDAB_DECL_CONSTEXPR KDRect KDRect::intersected( const KDRect & r ) const { return KDRect( kdMax( x1, r.x1 ), kdMax( y1, r.y1 ), kdMin( x2, r.x2 ), kdMin( y2, r.y2 ) ); }
inline KDRect KDRect::intersected( const QRect & r ) const { return intersected( reinterpret_cast<const KDRect&>( r ) ); }

inline KDAB_DECL_CONSTEXPR KDRect KDRect::united( const KDRect & r ) const { return KDRect( kdMin( x1, r.x1 ), kdMin( y1, r.y1 ), kdMax( x2, r.x2 ), kdMax( y2, r.y2 ) ); }
inline KDRect KDRect::united( const QRect & r ) const { return united( reinterpret_cast<const KDRect&>( r ) ); }

inline KDAB_DECL_CONSTEXPR KDRect KDRect::united( int x, int y ) const { return KDRect( kdMin( x1, x ), kdMin( y1, y ), kdMax( x2, x ), kdMax( y2, y ) ); }
inline KDRect KDRect::united( const QPoint & p ) const { return united( p.x(), p.y() ); }
inline KDAB_DECL_CONSTEXPR KDRect KDRect::united( const KDPoint & p ) const { return united( p.x(), p.y() ); }


inline KDAB_DECL_CONSTEXPR KDRect KDRect::movedBy( int dx, int dy ) const { return KDRect( x1 - dx, y1 - dy, x2 - dx, y2 - dy ); }
inline KDRect KDRect::movedBy( const QPoint & p ) const { return movedBy( p.x(), p.y() ); }
inline KDAB_DECL_CONSTEXPR KDRect KDRect::movedBy( const KDPoint & p ) const { return movedBy( p.x(), p.y() ); }

inline KDAB_DECL_CONSTEXPR KDRect KDRect::translated( int dx, int dy ) const { return movedBy( dx, dy ); }
inline KDRect KDRect::translated( const QPoint & p ) const { return movedBy( p ); }
inline KDAB_DECL_CONSTEXPR KDRect KDRect::translated( const KDPoint & p ) const { return movedBy( p ); }

inline KDRect KDRect::referencePointMoved( Qt::Alignment align, const QPoint & p ) const { return movedBy( p - referencePointImpl( align ) ); }
inline KDRECT_CONSTEXPR_FOR_QFLAGS KDRect KDRect::referencePointMoved( Qt::Alignment align, const KDPoint & p ) const { return movedBy( p - referencePointImpl( align ) ); }
inline KDAB_DECL_CONSTEXPR KDRect KDRect::referencePointMovedImpl( int align, const KDPoint & p ) const { return movedBy( p - referencePointImpl( align ) ); }
inline KDRECT_CONSTEXPR_FOR_QFLAGS KDRect KDRect::referencePointMoved( Qt::Alignment align, int x, int y ) const { return referencePointMoved( align, KDPoint( x, y ) ); }
inline KDRect KDRect::referencePointMoved( Qt::Corner corner, const QPoint & p ) const { return movedBy( p - referencePoint( corner ) ); }
inline KDAB_DECL_CONSTEXPR KDRect KDRect::referencePointMoved( Qt::Corner corner, const KDPoint & p ) const { return movedBy( p - referencePoint( corner ) ); }
inline KDAB_DECL_CONSTEXPR KDRect KDRect::referencePointMoved( Qt::Corner corner, int x, int y ) const { return referencePointMoved( corner, KDPoint( x, y ) ); }

inline KDRect KDRect::movedTo( const QPoint & tl ) const { return fromTopLeftAndSize( tl, width(), height() ); }
inline KDAB_DECL_CONSTEXPR KDRect KDRect::movedTo( const KDPoint & tl ) const { return fromTopLeftAndSize( tl, width(), height() ); }
inline KDAB_DECL_CONSTEXPR KDRect KDRect::movedTo( int l, int t ) const { return fromTopLeftAndSize( l, t, width(), height() ); }

inline KDAB_DECL_CONSTEXPR KDRect KDRect::topMoved( int t )     const { return fromTopLeftAndSize( left(), t, width(), height() ); }
inline KDAB_DECL_CONSTEXPR KDRect KDRect::leftMoved( int l )    const { return fromTopLeftAndSize( l, top(), width(), height() ); }
inline KDAB_DECL_CONSTEXPR KDRect KDRect::rightMoved( int r )   const { return fromTopRightAndSize( r, top(), width(), height() ); }
inline KDAB_DECL_CONSTEXPR KDRect KDRect::bottomMoved( int b )  const { return fromBottomLeftAndSize( left(), b, width(), height() ); }
inline KDAB_DECL_CONSTEXPR KDRect KDRect::vCenterMoved( int v ) const { return translated( 0, vCenter() - v ); }
inline KDAB_DECL_CONSTEXPR KDRect KDRect::hCenterMoved( int h ) const { return translated( hCenter() - h, 0 ); }

inline KDAB_DECL_CONSTEXPR KDRect KDRect::topLeftMoved( int l, int t )           const { return referencePointMoved( Qt::TopLeftCorner,       l, t ); }
inline KDRect KDRect::topLeftMoved( const QPoint & tl )      const { return topLeftMoved( tl.x(), tl.y() ); }
inline KDAB_DECL_CONSTEXPR KDRect KDRect::topLeftMoved( const KDPoint & tl )      const { return topLeftMoved( tl.x(), tl.y() ); }
inline KDAB_DECL_CONSTEXPR KDRect KDRect::topCenterMoved( int h, int t )         const { return referencePointMovedImpl( static_cast<int>(Qt::AlignTop)|static_cast<int>(Qt::AlignHCenter),    KDPoint( h, t ) ); }
inline KDRect KDRect::topCenterMoved( const QPoint & tc )    const { return topCenterMoved( tc.x(), tc.y() ); }
inline KDAB_DECL_CONSTEXPR KDRect KDRect::topCenterMoved( const KDPoint & tc )    const { return topCenterMoved( tc.x(), tc.y() ); }
inline KDAB_DECL_CONSTEXPR KDRect KDRect::topRightMoved( int r, int t )          const { return referencePointMoved( Qt::TopRightCorner,      r, t ); }
inline KDRect KDRect::topRightMoved( const QPoint & tr )     const { return topRightMoved( tr.x(), tr.y() ); }
inline KDAB_DECL_CONSTEXPR KDRect KDRect::topRightMoved( const KDPoint & tr )     const { return topRightMoved( tr.x(), tr.y() ); }
inline KDAB_DECL_CONSTEXPR KDRect KDRect::leftCenterMoved( int l, int v )        const { return referencePointMovedImpl( static_cast<int>(Qt::AlignVCenter)|static_cast<int>(Qt::AlignLeft),   KDPoint( l, v ) ); }
inline KDRect KDRect::leftCenterMoved( const QPoint & lc )   const { return leftCenterMoved( lc.x(), lc.y() ); }
inline KDAB_DECL_CONSTEXPR KDRect KDRect::leftCenterMoved( const KDPoint & lc )   const { return leftCenterMoved( lc.x(), lc.y() ); }
inline KDAB_DECL_CONSTEXPR KDRect KDRect::centerMoved( int h, int v )            const { return referencePointMovedImpl( Qt::AlignCenter,                  KDPoint( h, v ) ); }
inline KDRect KDRect::centerMoved( const QPoint & c )        const { return centerMoved( c.x(), c.y() ); }
inline KDAB_DECL_CONSTEXPR KDRect KDRect::centerMoved( const KDPoint & c )        const { return centerMoved( c.x(), c.y() ); }
inline KDAB_DECL_CONSTEXPR KDRect KDRect::rightCenterMoved( int r, int v )       const { return referencePointMovedImpl( static_cast<int>(Qt::AlignVCenter)|static_cast<int>(Qt::AlignRight),  KDPoint( r, v ) ); }
inline KDRect KDRect::rightCenterMoved( const QPoint & rc )  const { return rightCenterMoved( rc.x(), rc.y() ); }
inline KDAB_DECL_CONSTEXPR KDRect KDRect::rightCenterMoved( const KDPoint & rc )  const { return rightCenterMoved( rc.x(), rc.y() ); }
inline KDAB_DECL_CONSTEXPR KDRect KDRect::bottomLeftMoved( int l, int b )        const { return referencePointMoved( Qt::BottomLeftCorner,    l, b ); }
inline KDRect KDRect::bottomLeftMoved( const QPoint & bl )   const { return bottomLeftMoved( bl.x(), bl.y() ); }
inline KDAB_DECL_CONSTEXPR KDRect KDRect::bottomLeftMoved( const KDPoint & bl )   const { return bottomLeftMoved( bl.x(), bl.y() ); }
inline KDAB_DECL_CONSTEXPR KDRect KDRect::bottomCenterMoved( int h, int b )      const { return referencePointMovedImpl( static_cast<int>(Qt::AlignBottom)|static_cast<int>(Qt::AlignHCenter), KDPoint( h, b ) ); }
inline KDRect KDRect::bottomCenterMoved( const QPoint & bc ) const { return bottomCenterMoved( bc.x(), bc.y() ); }
inline KDAB_DECL_CONSTEXPR KDRect KDRect::bottomCenterMoved( const KDPoint & bc ) const { return bottomCenterMoved( bc.x(), bc.y() ); }
inline KDAB_DECL_CONSTEXPR KDRect KDRect::bottomRightMoved( int r, int b )       const { return referencePointMoved( Qt::BottomRightCorner,   r, b ); }
inline KDRect KDRect::bottomRightMoved( const QPoint & br )  const { return bottomRightMoved( br.x(), br.y() ); }
inline KDAB_DECL_CONSTEXPR KDRect KDRect::bottomRightMoved( const KDPoint & br )  const { return bottomRightMoved( br.x(), br.y() ); }

inline KDRECT_CONSTEXPR_FOR_QFLAGS KDRect KDRect::aligned( const KDRect & ref, Qt::Alignment align ) const { return referencePointMovedImpl( align, ref.referencePointImpl( align ) ); }
inline KDAB_DECL_CONSTEXPR KDRect KDRect::aligned( const KDRect & ref, Qt::Corner corner ) const { return referencePointMoved( corner, ref.referencePoint( corner ) ); }
inline KDRect KDRect::aligned( const QRect & ref, Qt::Alignment align ) const { return aligned( reinterpret_cast<const KDRect&>( ref ), align ); }
inline KDRect KDRect::aligned( const QRect & ref, Qt::Corner corner ) const { return aligned( reinterpret_cast<const KDRect&>( ref ), corner ); }

inline KDAB_DECL_CONSTEXPR KDRect KDRect::resized( int w, int h ) const { return fromTopLeftAndSize( left(), top(), w, h ); }
inline KDRect KDRect::resized( const QSize & sz ) const { return resized( sz.width(), sz.height() ); }

inline KDAB_DECL_CONSTEXPR KDRect KDRect::transposed() const { return fromTopLeftAndSize( left(), top(), height(), width() ); }

inline KDAB_DECL_CONSTEXPR KDRect KDRect::mirrored() const { return mirrored( 0, 0 ); }
inline KDAB_DECL_CONSTEXPR KDRect KDRect::mirrored( int x, int y ) const { return fromPoints( 2 * x - right(), 2 * y - bottom(), 2 * x - left(), 2 * y - top() ); }
inline KDRect KDRect::mirrored( const QPoint & p ) const { return mirrored( p.x(), p.y() ); }
inline KDAB_DECL_CONSTEXPR KDRect KDRect::mirrored( const KDPoint & p ) const { return mirrored( p.x(), p.y() ); }

inline KDAB_DECL_CONSTEXPR KDRect KDRect::adjusted( int dl, int dt, int dr, int db ) const { return fromPoints( left() + dl, top() + dt, right() + dr, bottom() + db ); }

inline KDAB_DECL_CONSTEXPR KDRect KDRect::grown( int delta ) const { return adjusted( -delta, -delta, delta, delta ); }
inline KDAB_DECL_CONSTEXPR KDRect KDRect::shrunken( int delta ) const { return grown( -delta ); }

inline KDRect KDRect::expandedTo( const QSize & sz ) const { return resized( size().expandedTo( sz ) ); }
inline KDRect KDRect::expandedTo( int w, int h ) const { return expandedTo( QSize( w, h ) ); }

inline KDRect KDRect::boundedTo( const QSize & sz ) const { return resized( size().boundedTo( sz ) ); }
inline KDRect KDRect::boundedTo( int w, int h ) const { return boundedTo( QSize( w, h ) ); }

#endif /* __KDTOOLSCORE_KDRECT_H__ */

