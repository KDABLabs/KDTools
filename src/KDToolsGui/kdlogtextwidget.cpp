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

#include "kdlogtextwidget.h"

#include <QBasicTimer>
#include <QHash>
#include <QPainter>
#include <QPaintEvent>
#include <QScrollBar>

#include <cassert>
#include <algorithm>
#include <iterator>

/*!
  \class KDLogTextWidget
  \ingroup gui
  \brief A high-speed text display widget.

  This widget provides very fast display of large amounts of
  line-oriented text, as commonly found in application log
  viewers. The feature set and implementation are optimized for
  frequent appends.

  You can set initial text using setLines(), and append lines with
  calls to message(). You can limit the number of lines kept in the
  view using setHistorySize().

  Text formatting is currently limited to per-line text color, but is
  expected to be enhanced on client request in upcoming versions. You
  can pass the color to use to calls to message().

  \image html kdlogtextwidget.png

  \sa KDLog
*/

class KDLogTextWidget::Private {
    friend class ::KDLogTextWidget;
    KDLogTextWidget * const q;
public:
    explicit Private( KDLogTextWidget * qq );
    ~Private();

    void updateCache() const;

    void triggerTimer() {
        if ( !timer.isActive() )
            timer.start( 500, q );
    }

    void addPendingLines();
    void enforceHistorySize();
    void updateScrollRanges();
    QPair<int,int> visibleLines( int top, int bottom ) {
        return qMakePair( qMax( 0, lineByYCoordinate( top ) ),
                          qMax( 0, 1 + lineByYCoordinate( bottom ) ) );
    }
    int lineByYCoordinate( int x ) const;

    QPoint scrollOffset() const;

    QRect lineRect( int idx ) const {
        assert( !cache.dirty );
        return QRect( 0, idx * cache.fontMetrics.lineSpacing, cache.dimensions.longestLineLength, cache.fontMetrics.lineSpacing-1 );
    }

    struct Style {
        QColor color;

        friend inline uint qHash( const Style & style ) {
            return qHash( style.color.rgba() );
        }
        bool operator==( const Style & other ) const { return this->color.rgba() == other.color.rgba(); }
        bool operator<( const Style & other ) const { return this->color.rgba() < other.color.rgba(); }
    };

    struct LineItem {
        QString text;
        unsigned int styleID;
    };

    unsigned int findOrAddStyle( const Style & style );

private:
    QHash<unsigned int, Style> styleByID;
    QHash<Style,unsigned int> idByStyle;

    QVector<LineItem> lines, pendingLines;

    unsigned int historySize;
    unsigned int minimumVisibleLines;
    unsigned int minimumVisibleColumns;

    bool alternatingRowColors;

    QBasicTimer timer;

    mutable struct Cache {
        enum { Dimensions = 1, FontMetrics = 2, All = FontMetrics|Dimensions };
        Cache() : dirty( All ) {}
        int dirty;

        struct {
            int lineSpacing;
            int ascent;
            int averageCharWidth;
            QVector<int> lineWidths;
        } fontMetrics;

        struct {
            int indexOfLongestLine;
            int longestLineLength;
        } dimensions;
    } cache;
};

/*!
  Constructor. Creates an empty KDLogTextWidget.
*/
KDLogTextWidget::KDLogTextWidget( QWidget * parent_ )
    : QAbstractScrollArea( parent_ ), d( new Private( this ) )
{

}

/*!
  Destructor.
*/
KDLogTextWidget::~KDLogTextWidget() {}


/*!
  \property KDLogTextWidget::historySize

  Specifies the maximum number of lines this widget will hold before
  dropping old lines. The default is INT_MAX (ie. essentially unlimited).

  Get this property's value using %historySize(), and set it with
  %setHistorySize().
*/
void KDLogTextWidget::setHistorySize( unsigned int hs ) {
    if ( hs == d->historySize )
        return;
    d->historySize = hs;
    d->enforceHistorySize();
    d->updateScrollRanges();
    viewport()->update();
}

unsigned int KDLogTextWidget::historySize() const {
    return d->historySize;
}

/*!
  \property KDLogTextWidget::text

  Contains the current %text as a single string. Equivalent to
  \code
  lines().join( "\n" )
  \endcode
*/
QString KDLogTextWidget::text() const {
    return lines().join( QLatin1String( "\n" ) );
}

/*!
  \property KDLogTextWidget::lines

  Contains the current %text as a string list. The default empty.

  Get this property's value using %lines(), and set it with
  %setLines().
*/
void KDLogTextWidget::setLines( const QStringList & l ) {
    clear();
    Q_FOREACH( const QString & s, l )
        message( s );
}

QStringList KDLogTextWidget::lines() const {
    QStringList result;
    Q_FOREACH( const Private::LineItem & li, d->lines )
        result.push_back( li.text );
    Q_FOREACH( const Private::LineItem & li, d->pendingLines )
        result.push_back( li.text );
    return result;
}

/*!
  \property KDLogTextWidget::minimumVisibleLines

  Specifies the number of lines that should be visible at any one
  time. The default is 1 (one).

  Get this property's value using %minimumVisibleLines(), and set it
  using %setMinimumVisibleLines().
*/
void KDLogTextWidget::setMinimumVisibleLines( unsigned int num ) {
    if ( num == d->minimumVisibleLines )
        return;
    d->minimumVisibleLines = num;
    updateGeometry();
}

unsigned int KDLogTextWidget::minimumVisibleLines() const {
    return d->minimumVisibleLines;
}

/*!
  \property KDLogTextWidget::minimumVisibleColumns

  \since_p 2.2

  Specifies the number of columns that should be visible at any one
  time. The default is 1 (one). The width is calculated using
  QFontMetrics::averageCharWidth(), if that is available. Otherwise,
  the width of \c M is used.

  Get this property's value using %minimumVisibleColumns(), and set it
  using %setMinimumVisibleColumns().
*/
void KDLogTextWidget::setMinimumVisibleColumns( unsigned int num ) {
    if ( num == d->minimumVisibleColumns )
        return;
    d->minimumVisibleColumns = num;
    updateGeometry();
}

unsigned int KDLogTextWidget::minimumVisibleColumns() const {
    return d->minimumVisibleColumns;
}

/*!
  \property KDLogTextWidget::alternatingRowColors

  \since_p 2.2

  Specifies whether the background should be drawn using
  row-alternating colors. The default is \c false.

  Get this property's value using %alternatingRowColors(), and set it
  using %setAlternatingRowColors().
*/
void KDLogTextWidget::setAlternatingRowColors( bool on ) {
    if ( on == d->alternatingRowColors )
        return;
    d->alternatingRowColors = on;
    update();
}

bool KDLogTextWidget::alternatingRowColors() const {
    return d->alternatingRowColors;
}

QSize KDLogTextWidget::minimumSizeHint() const {
    d->updateCache();
    const QSize base = QAbstractScrollArea::minimumSizeHint();
    const QSize view( d->minimumVisibleColumns * d->cache.fontMetrics.averageCharWidth,
                      d->minimumVisibleLines   * d->cache.fontMetrics.lineSpacing );
    const QSize scrollbars( verticalScrollBar() ? verticalScrollBar()->minimumSizeHint().width() : 0,
			    horizontalScrollBar() ? horizontalScrollBar()->minimumSizeHint().height() : 0 );
    return base + view + scrollbars;
}

QSize KDLogTextWidget::sizeHint() const {
    if ( d->minimumVisibleLines > 1 || d->minimumVisibleColumns > 1 )
        return minimumSizeHint();
    else
        return 2 * minimumSizeHint();
}

/*!
  Clears the text.

  \post lines().empty() == true
*/
void KDLogTextWidget::clear() {
    d->timer.stop();
    d->lines.clear();
    d->pendingLines.clear();
    d->styleByID.clear();
    d->idByStyle.clear();
    d->cache.dirty = Private::Cache::All;
    viewport()->update();
}

/*!
  Appends \a str to the view, highlighting the line in \a color.

  \post lines().back() == str (modulo trailing whitespace and contained newlines)
*/
void KDLogTextWidget::message( const QString & str, const QColor & color ) {
    const Private::Style s = { color };
    const Private::LineItem li = { str, d->findOrAddStyle( s ) };
    d->pendingLines.push_back( li );
    d->triggerTimer();
}

/*!
  \overload

  Uses the default text color set in this widget's palette.
*/
void KDLogTextWidget::message( const QString & str ) {
    const Private::LineItem li = { str, 0 };
    d->pendingLines.push_back( li );
    d->triggerTimer();
}

void KDLogTextWidget::paintEvent( QPaintEvent * e ) {

    d->updateCache();

    QPainter p( viewport() );

    p.translate( -d->scrollOffset() );

    const QRect visible = p.matrix().inverted().mapRect( e->rect() );

    const QPair<int,int> visibleLines
        = d->visibleLines( visible.top(), visible.bottom() );

    assert( visibleLines.first <= visibleLines.second );

    const Private::Style defaultStyle = { p.pen().color() };

    const Private::Cache & cache = d->cache;

    p.setPen( Qt::NoPen );

    p.setBrush( palette().base() );

    if ( d->alternatingRowColors ) {

        p.drawRect( visible );

#if 0 // leaves garbage
        for ( unsigned int i = visibleLines.first % 2 ? visibleLines.first+1 : visibleLines.first,   end = visibleLines.second ; i < end ; i+=2 )
            p.drawRect( d->lineRect( i ) );

        if ( visibleLines.second >= 0 ) {
            const int lastY = d->lineRect( visibleLines.second-1 ).y();
            if ( lastY < visible.bottom() )
                p.drawRect( 0, lastY+1, cache.dimensions.longestLineLength, visible.bottom() - lastY );
        }
#endif

        p.setBrush( palette().alternateBase() );
        for ( unsigned int i = visibleLines.first % 2 ? visibleLines.first   : visibleLines.first+1, end = visibleLines.second ; i < end ; i+=2 )
            p.drawRect( d->lineRect( i ) );

    } else {

        p.drawRect( visible );

    }

    // ### unused optimization: paint lines by styles to minimise pen changes.
    for ( unsigned int i = visibleLines.first, end = visibleLines.second ; i != end ; ++i ) {
        const Private::LineItem & li = d->lines[i];
        assert( !li.styleID || d->styleByID.contains( li.styleID ) );
        const Private::Style & st = li.styleID ? d->styleByID[li.styleID] : defaultStyle ;

        p.setPen( st.color );
        p.drawText( 0, i * cache.fontMetrics.lineSpacing + cache.fontMetrics.ascent, li.text );
    }

}

void KDLogTextWidget::timerEvent( QTimerEvent * e ) {
    if ( e->timerId() == d->timer.timerId() ) {
        d->addPendingLines();
        d->timer.stop();
    } else {
        QAbstractScrollArea::timerEvent( e );
    }
}

void KDLogTextWidget::changeEvent( QEvent * e ) {
    QAbstractScrollArea::changeEvent( e );
    d->cache.dirty |= Private::Cache::FontMetrics;
}

void KDLogTextWidget::resizeEvent( QResizeEvent * ) {
    d->updateScrollRanges();
}

KDLogTextWidget::Private::Private( KDLogTextWidget * qq )
    : q( qq ),
      styleByID(),
      idByStyle(),
      lines(),
      pendingLines(),
      historySize( 0xFFFFFFFF ),
      minimumVisibleLines( 1 ),
      minimumVisibleColumns( 1 ),
      alternatingRowColors( false ),
      timer(),
      cache()
{
    // PENDING(marc) find all the magic flags we need here...
    QWidget * const vp = qq->viewport();
    vp->setBackgroundRole( QPalette::Base );
    vp->setAttribute( Qt::WA_StaticContents );
    vp->setAttribute( Qt::WA_NoSystemBackground );
#ifndef QT_NO_CURSOR
    vp->setCursor(Qt::IBeamCursor);
#endif
}

KDLogTextWidget::Private::~Private() {}


void KDLogTextWidget::Private::updateCache() const {

    if ( cache.dirty >= Cache::FontMetrics ) {
        const QFontMetrics & fm = q->fontMetrics();
        cache.fontMetrics.lineSpacing = fm.lineSpacing();
        cache.fontMetrics.ascent = fm.ascent();
#if QT_VERSION < 0x040200
        cache.fontMetrics.averageCharWidth = fm.width( QLatin1Char( 'M' ) );
#else
        cache.fontMetrics.averageCharWidth = fm.averageCharWidth();
#endif

        QVector<int> & lw = cache.fontMetrics.lineWidths;
        lw.clear();
        lw.reserve( lines.size() );
        Q_FOREACH( const LineItem & li, lines )
            lw.push_back( fm.width( li.text ) );
    }

    if ( cache.dirty >= Cache::Dimensions ) {
        const QVector<int> & lw = cache.fontMetrics.lineWidths;
        const QVector<int>::const_iterator it =
            std::max_element( lw.begin(), lw.end() );
        if ( it == lw.end() ) {
            cache.dimensions.indexOfLongestLine = -1;
            cache.dimensions.longestLineLength = 0;
        } else {
            cache.dimensions.indexOfLongestLine = it - lw.begin();
            cache.dimensions.longestLineLength = *it;
        }
    }

    cache.dirty = false;
}

unsigned int KDLogTextWidget::Private::findOrAddStyle( const Style & s ) {
    if ( idByStyle.contains( s ) ) {
        const unsigned int id = idByStyle[s];
        assert( styleByID.contains( id ) );
        assert( styleByID[id] == s );
        return id;
    } else {
        static unsigned int nextID = 0; // remember, 0 is reserved
        const unsigned int id = ++nextID;
        idByStyle.insert( s, id );
        styleByID.insert( id, s );
        return id;
    }
}

void KDLogTextWidget::Private::enforceHistorySize() {
    const size_t numLimes = lines.size();
    if ( numLimes <= historySize )
        return;
    const int remove = numLimes - historySize ;
    lines.erase( lines.begin(), lines.begin() + remove );

    // can't quickly update the dimensions if the fontMetrics aren't uptodate.
    if ( cache.dirty & Cache::FontMetrics ) {
        cache.dirty |= Cache::Dimensions;
        return;
    }

    QVector<int> & lw = cache.fontMetrics.lineWidths;

    assert( lw.size() > remove );
    lw.erase( lw.begin(), lw.begin() + remove );

    if ( cache.dirty & Cache::Dimensions )
        return;

    if ( cache.dimensions.indexOfLongestLine >= remove )
        cache.dimensions.indexOfLongestLine -= remove;
    else
        cache.dirty |= Cache::Dimensions;
}

static void set_scrollbar_properties( QScrollBar & sb, int document, int viewport, int singleStep, Qt::Orientation o ) {
    const int min = 0;
    const int max = std::max( 0, document - viewport );
    const int value = sb.value();
    const bool wasAtEnd = value == sb.maximum();
    sb.setRange( min, max );
    sb.setPageStep( viewport );
    sb.setSingleStep( singleStep );
    sb.setValue( o == Qt::Vertical && wasAtEnd ? sb.maximum() : value );
}

void KDLogTextWidget::Private::updateScrollRanges() {

    updateCache();

    if ( QScrollBar * const sb = q->verticalScrollBar() ) {
        const int document = lines.size() * cache.fontMetrics.lineSpacing ;
        const int viewport = q->viewport()->height();
        const int singleStep = cache.fontMetrics.lineSpacing;
        set_scrollbar_properties( *sb, document, viewport, singleStep, Qt::Vertical );
    }

    if ( QScrollBar * const sb = q->horizontalScrollBar() ) {
        const int document = cache.dimensions.longestLineLength;
        const int viewport = q->viewport()->width();
        const int singleStep = cache.fontMetrics.lineSpacing; // rather randomly chosen
        set_scrollbar_properties( *sb, document, viewport, singleStep, Qt::Horizontal );
    }
}

void KDLogTextWidget::Private::addPendingLines() {
    if ( pendingLines.empty() )
        return;

    const unsigned int oldNumLines = lines.size();

    lines += pendingLines;

    // if the cache isn't dirty, we can quickly update it without
    // invalidation:

    if ( !cache.dirty ) {

        // update fontMetrics:
        const QFontMetrics & fm = q->fontMetrics();
        QVector<int> plw;
        plw.reserve( pendingLines.size() );
        Q_FOREACH( const LineItem & li, pendingLines )
            plw.push_back( fm.width( li.text ) );

        // update dimensions:
        const QVector<int>::const_iterator it =
            std::max_element( plw.begin(), plw.end() );
        if ( *it >= cache.dimensions.longestLineLength ) {
            cache.dimensions.longestLineLength = *it;
            cache.dimensions.indexOfLongestLine = oldNumLines + ( it - plw.begin() );
        }
    }

    pendingLines.clear();

    enforceHistorySize();
    updateScrollRanges();
    q->viewport()->update();
}

int KDLogTextWidget::Private::lineByYCoordinate( int y ) const {
    updateCache();
    if ( cache.fontMetrics.lineSpacing == 0 )
        return -1;
    const int raw = y / cache.fontMetrics.lineSpacing ;
    if ( raw < 0 )
        return -1;
    if ( raw >= lines.size() )
        return lines.size() - 1;
    return raw;
}

static int get_scrollbar_offset( const QScrollBar * sb ) {
    return sb ? sb->value() : 0 ;
}

QPoint KDLogTextWidget::Private::scrollOffset() const {
    return QPoint( get_scrollbar_offset( q->horizontalScrollBar() ),
		   get_scrollbar_offset( q->verticalScrollBar() ) );
}

#include "moc_kdlogtextwidget.cpp"
