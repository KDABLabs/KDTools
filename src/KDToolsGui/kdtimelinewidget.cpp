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

#include "kdtimelinewidget.h"
#include "kdtimescrollbar.h"

#include <QDateTime>
#include <QEvent>
#include <QMouseEvent>
#include <QStyleOption>
#include <QStylePainter>
#include <QVBoxLayout>

#include <algorithm>
#include <functional>

/*!
  \defgroup KDTimeLineWidget KDTimeLineWidget
  \ingroup gui
*/

/* Sigh! MSVC6 lower_bound and upper_bound are
   not working with Qt iterators.
   Let's try to substitute for our own:
*/

namespace {
template<typename ForwardIter, typename BoundType, typename Compare>
ForwardIter kd_lower_bound( ForwardIter first, ForwardIter last,
                            const BoundType& val, Compare cmp )
{
  typedef typename ForwardIter::difference_type DistanceType;

  DistanceType len = last-first;
  DistanceType half;
  ForwardIter middle;

  while(len > 0) {
    half = len >> 1;
    middle = first;
    middle += half;
    if(cmp(*middle, val)) {
      first = middle;
      ++first;
      len = len - half - 1;
    } else
      len = half;
  }
  return first;
}

template<typename ForwardIter, typename BoundType, typename Compare>
ForwardIter kd_upper_bound( ForwardIter first, ForwardIter last,
                            const BoundType& val, Compare cmp )
{
  typedef typename ForwardIter::difference_type DistanceType;

  DistanceType len = last-first;
  DistanceType half;
  ForwardIter middle;

  while(len > 0) {
    half = len >> 1;
    middle = first;
    middle += half;
    if(cmp(val,*middle)) {
      len = half;
    } else {
      first = middle;
      ++first;
      len = len - half - 1;
    }
  }
  return first;
}
}

namespace {
  static qint64 datetime_to_usecs( const QDateTime& dt )
  {
    qint64 result;
    result = QDate( 1970, 1, 1 ).daysTo(dt.date())*(Q_INT64_C(24)*Q_INT64_C(60)*Q_INT64_C(60)*Q_INT64_C(1000));
    result += QTime( 0,0,0,0 ).msecsTo(dt.time());
    return result;
  }
  static QDateTime usecs_to_datetime( qint64 t )
  {
    return QDateTime( QDate(1970,1,1).addDays(t/(Q_INT64_C(24)*Q_INT64_C(60)*Q_INT64_C(60)*Q_INT64_C(1000))),
                      QTime(0,0,0,0).addMSecs(t%(Q_INT64_C(24)*Q_INT64_C(60)*Q_INT64_C(60)*Q_INT64_C(1000))) );
  }
}

/***********************************************
 *               KDTimeLineWidgetItem
 **********************************************/

/* internal */
struct KDTimeLineWidgetItemLessThan {
  bool operator()( const KDTimeLineWidgetItem* item1, const KDTimeLineWidgetItem* item2 ) const
  {
    return( item1->dateTime() < item2->dateTime() );
  }
  bool operator()( const KDTimeLineWidgetItem* item1, const QDateTime& dt ) const
  {
    return( item1->dateTime() < dt );
  }
  bool operator()( const QDateTime& dt, const KDTimeLineWidgetItem* item2 ) const
  {
    return( dt < item2->dateTime() );
  }
};

class KDTimeLineWidgetItem::Private {
    friend class ::KDTimeLineWidgetItem;
    KDTimeLineWidgetItem * const q;
public:
    explicit Private( KDTimeLineWidgetItem * qq );
    ~Private() {}

    void setTimeLineWidget( KDTimeLineWidget * t ) {
        timeLineWidget = t;
    }

private:
    KDTimeLineWidget * timeLineWidget;
    QDateTime dateTime;
    QPalette palette;
    bool movable;
};

KDTimeLineWidgetItem::Private::Private( KDTimeLineWidgetItem * qq )
    : q( qq ),
      timeLineWidget( 0 ),
      dateTime( QDateTime::currentDateTime() ),
      movable( true )
{

}

/*! \class KDTimeLineWidgetItem KDTimeLineWidgetItem
  \ingroup KDTimeLineWidget
 \brief Objects of this class are used as event items in
 a KDTimeLineWidget. It is possible to use the class as is,
 and it can be subclassed for further specialization.
*/

/*!
  Constructor. Creates an instance of a KDTimeLineWidgetItem
  positioned at the current time (QDateTime::currentDateTime()).
*/
KDTimeLineWidgetItem::KDTimeLineWidgetItem()
  : d( new Private( this ) )
{

}

/*! Constructor. Creates an instance of a KDTimeLineWidgetItem
  positioned at time \a dt */
KDTimeLineWidgetItem::KDTimeLineWidgetItem( const QDateTime& dt )
  : d( new Private( this ) )
{
    d->dateTime = dt;
}

/*! Destructor. If this KDTimeLineWidgetItem is owned
  by a KDTimeLineWidget, it will be removed from the KDTimeLineWidget. */
KDTimeLineWidgetItem::~KDTimeLineWidgetItem()
{
  if( d->timeLineWidget ) d->timeLineWidget->removeItem(this);
}

/*! Set the datetime the event item will be displayed on. */
void KDTimeLineWidgetItem::setDateTime( const QDateTime& dt )
{
  d->dateTime = dt;
  KDTimeLineWidget* t = timeLineWidget();
  if( t ) {
    t->removeItem(this);
    t->addItem(this);
  }
}

/*! \returns the datetime the event item will be displayed on */
QDateTime KDTimeLineWidgetItem::dateTime() const { return d->dateTime; }

/*! Assigns a custom palette \a pal to this item. */
void KDTimeLineWidgetItem::setPalette( const QPalette& pal )
{
  d->palette = pal;
  if( d->timeLineWidget ) d->timeLineWidget->update();
}

/*! \returns the palette used by this item. */
const QPalette& KDTimeLineWidgetItem::palette() const
{
  return d->palette;
}

/*! \returns true if this item is the current item,
  otherwise false.
*/
bool KDTimeLineWidgetItem::isCurrent() const
{
  if( !timeLineWidget() ) return false;
  return( timeLineWidget()->currentItem() == this );
}

/*!
  Configure the item to be movable by the user or
  not. Default is true.
*/
void KDTimeLineWidgetItem::setMovable( bool on )
{
    d->movable = on;
}

/*!
  Returns true if the item is movable, false otherwise.
*/
bool KDTimeLineWidgetItem::isMovable() const
{
    return d->movable;
}

/*!
  Returns a pointer to the KDTimeLineWidget widget that owns this
  item or 0 if the item is not owned by any KDTimeLineWidget
*/
KDTimeLineWidget * KDTimeLineWidgetItem::timeLineWidget() const
{
    return d->timeLineWidget;
}

/*! Paints the item at position \a pos using the painter
  \a p. Override this method to create KDTimeLineWidgetItems
  with different visual appearance */
void KDTimeLineWidgetItem::paintItem( QPainter* p, const QPoint& pos )
{
  p->save();
  p->translate(pos);
  if( isCurrent() ) {
    p->setBrush( palette().brush( QPalette::Active, QPalette::Background ) );
    p->setPen( palette().color( QPalette::Active, QPalette::Foreground ) );
  } else {
    p->setBrush( palette().brush( QPalette::Inactive, QPalette::Background ) );
    p->setPen( palette().color( QPalette::Inactive, QPalette::Foreground ) );
  }
  p->setRenderHint( QPainter::Antialiasing );
  QPolygon poly; /* TODO: Nicer drawing */
  poly.setPoints( 5,
                  -5,0,
                  5,0,
                  5,9,
                  0,18,
                  -5,9);
  p->drawConvexPolygon( poly );
  p->restore();
}

/***********************************************
 *               KDTimeLineWidget
 **********************************************/

/*! \class KDTimeLineWidget KDTimeLineWidget
  \ingroup KDTimeLineWidget
  \brief KDTimeLineWidget is a widget showing a timeline with a current
  time and optionally a number of events. It does
  not provide scrollbars or other means of navigation.

  \image HTML kdtimelinewidget.png

*/

class KDTimeLineWidget::Private {
    friend class ::KDTimeLineWidget;
    KDTimeLineWidget * const q;
public:
    Private( KDTimeLineWidget * qq )
        : q( qq ),
          starttime( QDateTime::currentDateTime() ),
          endtime( starttime.addDays(1) ),
          date_format( Qt::TextDate ),
          snaptoticks(true),
          currentitem(0) {}

    ~Private() {}

private:
    int timeToView( qint64 ) const;
    qint64 viewToTime( int ) const;
    QString dateTimeToString( const QDateTime& ) const;
    void getTickmarkValues( QList< qint64 >& major, QList< qint64 >& minor, int minw ) const;

    QDateTime snapTo( int x );

private:
    QDateTime starttime;
    QDateTime endtime;
    QDateTime currenttime;
    QDateTime elapsedtime;

    QString date_format_str;
    Qt::DateFormat date_format;
    bool snaptoticks;

    QList<KDTimeLineWidgetItem*> items;
    KDTimeLineWidgetItem* currentitem;
};

/*! Contructor. Creates a KDTimeLineWidget with parent \a parent
  showing time from now and one day forward. */
KDTimeLineWidget::KDTimeLineWidget( QWidget* parent )
  : QWidget(parent),
    d( new Private( this ) )
{
  setFocusPolicy( Qt::StrongFocus );
  setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Fixed );
}

/*! Destructor. When a KDTimeLineWidget is deleted, any
  KDTimeLineWidgetItems contained in it are deleted too. */
KDTimeLineWidget::~KDTimeLineWidget()
{
    qDeleteAll( d->items );
    //while( !items.isEmpty() ) delete items.front();
}

/*! Set the starting time of the time line to \a dt*/
void KDTimeLineWidget::setStartDateTime( const QDateTime& dt )
{
  d->starttime = dt;
  if( d->starttime > d->endtime ) d->endtime = d->starttime;
  update();
}

/*! \fn KDTimeLineWidget::startDateTime() const
  \returns the oldest point in time displayed by this KDTimeLineWidget.
  \see KDTimeLineWidget::setStartDateTime( const QDateTime& )
*/

QDateTime KDTimeLineWidget::startDateTime() const { return d->starttime; }


/*! Set the ending time of the time line to \a dt*/
void KDTimeLineWidget::setEndDateTime( const QDateTime& dt )
{
  d->endtime = dt;
  if( d->starttime > d->endtime ) d->starttime = d->endtime;
  update();
}

/*! \fn KDTimeLineWidget::endDateTime() const
  \returns the youngest point in time displayed by this KDTimeLineWidget.
  \see KDTimeLineWidget::setEndDateTime( const QDateTime& )
*/
QDateTime KDTimeLineWidget::endDateTime() const { return d->endtime; }

/*! Set the current time of the timeline to \a dt.
  Current time display can be disabled by passing
  an invalid QDateTime to this method. */
void KDTimeLineWidget::setCurrentDateTime( const QDateTime& dt )
{
  QDateTime old = d->currenttime;
  d->currenttime = dt;
  if( old != dt ) emit currentDateTimeChanged(dt);
  update();
}

/*! \fn KDTimeLineWidget::currentDateTime() const
  \returns the datetime of the current time indicator.
*/
QDateTime KDTimeLineWidget::currentDateTime() const { return d->currenttime; }

/*! Set the elapsed time of the timeline to \a dt.
  Current time display can be disabled by passing
  an invalid QDateTime to this method. */
void KDTimeLineWidget::setElapsedDateTime( const QDateTime& dt )
{
  QDateTime old = d->elapsedtime;
  d->elapsedtime = dt;
  if( old != dt ) emit elapsedDateTimeChanged(dt);
  update();
}

/*! \fn KDTimeLineWidget::elapsedTime() const
  \returns the elapsed time.
  \see KDTimeLineWidget::setElapsedDateTime( const QDateTime& )
*/
QDateTime KDTimeLineWidget::elapsedDateTime() const { return d->elapsedtime; }

/*! Set the datetime format string used to display
 textual datetimes on the timeline. Default is QString::null
which means that the Qt::DateFormat set with
setDateTimeFormat( Qt::DateFormat ) is used instead. */
void KDTimeLineWidget::setDateTimeFormat( const QString& format )
{
  d->date_format_str = format;
  update();
}

/*! Set the datetime format string used to display
  textual datetimes on the timeline. Default is Qt::TextFormat.
  Calling this method sets the datime format string to
  QString::null. */
void KDTimeLineWidget::setDateTimeFormat( Qt::DateFormat format )
{
  d->date_format = format;
  d->date_format_str.clear();
  update();
}

QString KDTimeLineWidget::Private::dateTimeToString( const QDateTime& dt ) const
{
  if( date_format_str.isNull() ) return dt.toString( date_format );
  else return dt.toString( date_format_str );
}

/*! Add a KDTimeLineWidgetItem to this KDTimeLineWidget. The
  KDTimeLineWidget object becomes the owner of \a item */
void KDTimeLineWidget::addItem( KDTimeLineWidgetItem* item )
{
  Q_ASSERT(item);
  QList<KDTimeLineWidgetItem*>::iterator it = kd_lower_bound( d->items.begin(),
                                                        d->items.end(),
                                                        item,
                                                        KDTimeLineWidgetItemLessThan() );
  d->items.insert(it,item);
  item->d->setTimeLineWidget(this);
  update();
}

/*! Remove a KDTimeLineWidgetItem from this KDTimeLineWidget. Ownership
  is transferred to the caller and the item is \em not deleted.
*/
void KDTimeLineWidget::removeItem( KDTimeLineWidgetItem* item )
{
  Q_ASSERT(item);
  d->items.removeAll(item);
  item->d->setTimeLineWidget(0);
  if( d->currentitem == item ) d->currentitem = 0;
  update();
}

/*! \returns the current item. */
KDTimeLineWidgetItem* KDTimeLineWidget::currentItem() const
{
  return d->currentitem;
}

/*! Sets the current items to \a item. \a item must be an item
  on this timeline.
*/
void KDTimeLineWidget::setCurrentItem( KDTimeLineWidgetItem* item )
{
  //KDTimeLineWidgetItem* old = d->currentitem;
  d->currentitem = item;
  update();
}

/*! Advances the current time indicator to the next minor tickmark. */
void KDTimeLineWidget::nextCurrentDateTime()
{
    const QFontMetrics fm = fontMetrics();
    const int minw = fm.width( d->dateTimeToString( d->starttime) ) + fm.width( QLatin1String( "XXX" ) );

    QList< qint64 > major;
    QList< qint64 > minor;
    d->getTickmarkValues( major, minor, minw );

    if( minor.isEmpty() )
        return;

    const qint64 timex = datetime_to_usecs( currentDateTime() );
    const QList< qint64 >::const_iterator bound = kd_upper_bound( minor.begin(), minor.end(), timex, std::less< qint64 >() );
    if( bound != minor.end() )
        setCurrentDateTime( usecs_to_datetime( *bound ) );
    else
        return setCurrentDateTime( usecs_to_datetime( minor.last() ) );
}

/*! Advances the current time indicator to the previous minor tickmark. */
void KDTimeLineWidget::previousCurrentDateTime()
{
    const QFontMetrics fm = fontMetrics();
    const int minw = fm.width( d->dateTimeToString( d->starttime) ) + fm.width( QLatin1String( "XXX" ) );

    QList< qint64 > major;
    QList< qint64 > minor;
    d->getTickmarkValues( major, minor, minw );

    if( minor.isEmpty() )
        return;

    const qint64 timex = datetime_to_usecs( currentDateTime() );
    const QList< qint64 >::const_iterator bound = kd_lower_bound( minor.begin(), minor.end(), timex, std::less< qint64 >() );
    if( bound != minor.begin() )
        setCurrentDateTime( usecs_to_datetime( *( bound - 1 ) ) );
    else
        return setCurrentDateTime( usecs_to_datetime( minor.first() ) );
}

/*! Sets the next item on the timeline to be current. Does nothing
  if there is no next item */
void KDTimeLineWidget::nextCurrentItem()
{
  if( d->items.isEmpty() ) return;
  if( !currentItem() ) {
    setCurrentItem( d->items.front() );
  } else {
    /* TODO: This can be optimized */
    int idx = d->items.indexOf(currentItem());
    if( idx >= 0 && idx < d->items.size()-1 ) setCurrentItem( d->items.at(++idx) );
  }
}

/*! Sets the previous item on the timeline to be current. Does nothing
  if there is no previous item */
void KDTimeLineWidget::previousCurrentItem()
{
  if( d->items.isEmpty() ) return;
  if( !currentItem() ) {
    setCurrentItem( d->items.front() );
  } else {
    /* TODO: This can be optimized */
    int idx = d->items.indexOf(currentItem());
    if( idx > 0 ) setCurrentItem( d->items.at(--idx) );
  }
}

/*!\fn int KDTimeLineWidget::itemCount() const
  Returns the number of KDTimeLineWidgetItems.
*/
int KDTimeLineWidget::itemCount() const { return d->items.size(); }

/*!\fn KDTimeLineWidgetItem* KDTimeLineWidget::item(int idx)
  Returns a pointer to the KDTimeLineWidgetItem number
  \a idx in this widget.
 */
KDTimeLineWidgetItem* KDTimeLineWidget::item(int idx) const { return d->items.at(idx); }

/*! Returns the datetime associated with the given x
  coordinate in the widget */
QDateTime KDTimeLineWidget::dateTimeAt( int x ) const
{
  return usecs_to_datetime( d->viewToTime( x ) );
}

/*! \returns the position x coordinate in widget coordinates
  corresponding to the time \a dt */
int KDTimeLineWidget::position( const QDateTime& dt ) const
{
  return d->timeToView( datetime_to_usecs(dt) );
}

int KDTimeLineWidget::Private::timeToView( qint64 t ) const
{
  qint64 w = q->width();
  qint64 st = datetime_to_usecs( starttime );
  qint64 et = datetime_to_usecs( endtime );
  return w*(t-st)/(et-st);
}

qint64 KDTimeLineWidget::Private::viewToTime( int x ) const
{
  qint64 w = q->width();
  qint64 st = datetime_to_usecs( starttime );
  qint64 et = datetime_to_usecs( endtime );
  return x*(et-st)/w+st;
}

namespace
{
    template< typename T >
    struct DifferenceSmallerThanNotEqual
    {
        DifferenceSmallerThanNotEqual( const T& value, const T& diff )
            : value( value ),
              diff( diff )
        {
        }

        bool operator()( const T& v )
        {
            return qAbs( v - value ) < diff && value - v != static_cast< T >( 0 );
        }

    private:
        const T value;
        const T diff;
    };
}

void KDTimeLineWidget::Private::getTickmarkValues( QList< qint64 >& major, QList< qint64 >& minor, int minw ) const
{
    static const int yeardeltas[] = {   1,
                                        2,
                                        5,
                                       10,
                                       20,
                                       50,
                                       100,
                                       200,
                                       500,
                                      1000,
                                         0 };

    static const int monthdeltas[] = { 1,
                                       2,
                                       3,
                                       6,
                                       0 };

    static const int daydeltas[] = {  1,
                                      2,
                                      7,
                                     14,
                                      0 };

    static const int seconddeltas[] = {   1, // 1 msec
                                          2, // 2 msecs
                                          5, // 5 msecs
                                         10, // 10 msecs
                                         20, // 20 msecs
                                         50, // 50 msecs
                                        100, // 100 msecs
                                        200, // 200 msecs
                                        500, // 500 msecs
                                         1 * 1000, // 1 second
                                         2 * 1000, // 2 seconds
                                         5 * 1000, // 5 seconds
                                        10 * 1000, // 10 seconds
                                        15 * 1000, // 15 seconds
                                        30 * 1000, // 30 seconds
                                         1 * 60 * 1000, // 1 minute
                                         2 * 60 * 1000, // 2 minutes
                                         5 * 60 * 1000, // 5 minutes
                                        10 * 60 * 1000, // 10 minutes
                                        20 * 60 * 1000, // 20 minutes
                                        30 * 60 * 1000, // 30 minutes
                                         1 * 60 * 60 * 1000, // 1 hour
                                         2 * 60 * 60 * 1000, // 2 hours
                                         3 * 60 * 60 * 1000, // 3 hours
                                         6 * 60 * 60 * 1000, // 6 hours
                                        12 * 60 * 60 * 1000, // 12 hours
                                        0 };



    const qint64 st = datetime_to_usecs( starttime );
    const qint64 et = datetime_to_usecs( endtime );
    const qint64 usecpertick = ( et-st ) * minw / q->width();

    const qint64 left = st - minw / 2;
    const qint64 right = et + minw / 2;

    major.clear();
    minor.clear();

    QList< qint64 > years;
    const int beginy = starttime.date().year();
    const int endy = endtime.date().year();

    const int begin = beginy;
    const int end = endy;

    // first we try to put in complete year ticks (1000, 500, 200, 100, 50, 20, 10, 5, 2, 1)
    for( int delta = 0; yeardeltas[ delta ] != 0; ++delta )
    {
        const int d = yeardeltas[ delta ];
        const int begin = beginy - beginy % d;
        const int end = endy + d - ( endy % d );

        const qint64 first = datetime_to_usecs( QDateTime( QDate( begin, 1, 1 ) ) );
        const qint64 second = datetime_to_usecs( QDateTime( QDate( begin + d, 1, 1 ) ) );

        if( second - first < usecpertick )
            continue;

        years.clear();

        bool allFit = true;

        for( QDateTime t( QDate( begin, 1, 1 ) ); t.date().year() <= end; t = t.addYears( d ) )
        {
            const qint64 dt = datetime_to_usecs( t );
            if( dt < left || dt > right )
                continue;
            else if( std::find_if( years.begin(), years.end(), DifferenceSmallerThanNotEqual< qint64 >( dt, usecpertick ) ) == years.end() )
                years.push_back( dt );
            else
            {
                years.clear();
                t = QDateTime( QDate( end + 1, 1, 1 ) );
                allFit = false;
            }
        }

        major += years;

        if( allFit )
        {
            minor.clear();
            if( d == 1 )
            {
                for( QDateTime t( QDate( begin, 1, 1 ) ); t.date().year() <= end; t = t.addMonths( 1 ) )
                    minor.push_back( datetime_to_usecs( t ) );
            }
            else if( d <= 10 )
            {
                for( QDateTime t( QDate( begin, 1, 1 ) ); t.date().year() <= end; t = t.addYears( 1 ) )
                    minor.push_back( datetime_to_usecs( t ) );
            }
            else if( d <= 100 )
            {
                for( QDateTime t( QDate( begin, 1, 1 ) ); t.date().year() <= end; t = t.addYears( 10 ) )
                    minor.push_back( datetime_to_usecs( t ) );
            }
            else
            {
                for( QDateTime t( QDate( begin, 1, 1 ) ); t.date().year() <= end; t = t.addYears( 100 ) )
                    minor.push_back( datetime_to_usecs( t ) );
            }
            break;
        }
    }

    // then we try to put in complete month ticks (6, 3, 2, 1, 0)
    for( int delta = 0; monthdeltas[ delta ] != 0; ++delta )
    {
        const int d = monthdeltas[ delta ];

        const qint64 first = datetime_to_usecs( QDateTime( QDate( beginy, 1, 1 ) ) );
        const qint64 second = datetime_to_usecs( QDateTime( QDate( beginy, 1 + d, 1 ) ) );

        if( second - first < usecpertick )
            continue;

        years.clear();

        bool allFit = true;

        for( QDateTime t( QDate( begin, 1, 1 ) ); t.date().year() <= end; t = t.addMonths( d ) )
        {
            const qint64 dt = datetime_to_usecs( t );
            if( dt < left || dt > right )
                continue;
            else if( std::find_if( years.begin(), years.end(), DifferenceSmallerThanNotEqual< qint64 >( dt, usecpertick ) ) == years.end() )
                years.push_back( dt );
            else
            {
                years.clear();
                t = QDateTime( QDate( end + 1, 1, 1 ) );
                allFit = false;
            }
        }

        major += years;

        if( allFit )
        {
            minor.clear();
            if( d == 1 )
            {
                for( QDateTime t( QDate( begin, 1, 1 ) ); t.date().year() <= end; t = t.addDays( 1 ) )
                    minor.push_back( datetime_to_usecs( t ) );
            }
            else
            {
                for( QDateTime t( QDate( begin, 1, 1 ) ); t.date().year() <= end; t = t.addMonths( 1 ) )
                    minor.push_back( datetime_to_usecs( t ) );
            }
            break;
        }
    }

    // then we try to put in complete day ticks (7, 1, 0)
    for( int delta = 0; daydeltas[ delta ] != 0; ++delta )
    {
        const int d = daydeltas[ delta ];

        const qint64 first = datetime_to_usecs( QDateTime( QDate( beginy, 1, 1 ) ) );
        const qint64 second = datetime_to_usecs( QDateTime( QDate( beginy, 1, 1 + d ) ) );

        if( second - first < usecpertick )
            continue;

        const QDateTime begin = QDateTime( usecs_to_datetime( left ).date() ).addDays( -d );
        const QDateTime end = QDateTime( usecs_to_datetime( right ).date() ).addDays( d );

        years.clear();

        bool allFit = true;

        for( QDateTime t = begin; t <= end; t = t.addDays( d ) )
        {
            if( ( t.date().day() - 1 ) % d != 0 )
                t = QDateTime( QDate( t.date().year(), t.date().month(), 1 ) );

            if( t.date().day() > t.date().daysInMonth() + 1 - d )
                continue;

            const qint64 dt = datetime_to_usecs( t );
            if( std::find_if( years.begin(), years.end(), DifferenceSmallerThanNotEqual< qint64 >( dt, usecpertick ) ) == years.end() )
                years.push_back( dt );
            else
                allFit = false;
        }

        major += years;

        if( allFit )
        {
            minor.clear();
            if( d == 1 )
            {
                for( QDateTime t = begin; t <= end; t = t.addSecs( 60 * 60 ) )
                    minor.push_back( datetime_to_usecs( t ) );
            }
            else
            {
                for( QDateTime t = begin; t <= end; t = t.addDays( 1 ) )
                    minor.push_back( datetime_to_usecs( t ) );
            }
            break;
        }
    }

    // finally we try to put in complete milliseconds (up to hours...)
   for( int delta = 0; seconddeltas[ delta ] != 0; ++delta )
    {
        const int d = seconddeltas[ delta ];

        const qint64 first = datetime_to_usecs( QDateTime( QDate( beginy, 1, 1 ) ) );
        const qint64 second = datetime_to_usecs( QDateTime( QDate( beginy, 1, 1 ) ).addMSecs( d ) );

        if( second - first < usecpertick )
            continue;

        QDateTime begin = usecs_to_datetime( left ).addMSecs( -d );
        const int msecs = QTime().msecsTo( begin.time() );
        if( msecs % d != 0 )
            begin = QDateTime( begin.date(), QTime().addMSecs( msecs - msecs % d ) );

        const QDateTime end = usecs_to_datetime( right ).addMSecs( d );

        years.clear();

        bool allFit = true;

        for( QDateTime t = begin; t <= end; t = t.addMSecs( d ) )
        {
            const int msecs = QTime().msecsTo( t.time() );
            if( msecs % d != 0 )
                t = QDateTime( t.date(), QTime().addMSecs( msecs - msecs % d ) );

            const qint64 dt = datetime_to_usecs( t );
            if( std::find_if( years.begin(), years.end(), DifferenceSmallerThanNotEqual< qint64 >( dt, usecpertick ) ) == years.end() )
                years.push_back( dt );
            else
            {
                years.clear();
                t = end;
                allFit = false;
            }
        }

        major += years;
        if( allFit )
        {
            minor.clear();
            if( d <= 10 )
            {
                for( QDateTime t = begin; t <= end; t = t.addMSecs( 1 ) )
                    minor.push_back( datetime_to_usecs( t ) );
            }
            else if( d <= 100 )
            {
                for( QDateTime t = begin; t <= end; t = t.addMSecs( 10 ) )
                    minor.push_back( datetime_to_usecs( t ) );
            }
            else if( d <= 1000 )
            {
                for( QDateTime t = begin; t <= end; t = t.addMSecs( 100 ) )
                    minor.push_back( datetime_to_usecs( t ) );
            }
            else if( d <= 10 * 1000 )
            {
                for( QDateTime t = begin; t <= end; t = t.addSecs( 1 ) )
                    minor.push_back( datetime_to_usecs( t ) );
            }
            else if( d <= 60 * 1000 )
            {
                for( QDateTime t = begin; t <= end; t = t.addSecs( 10 ) )
                    minor.push_back( datetime_to_usecs( t ) );
            }
            else if( d <= 10 * 60 * 1000 )
            {
                for( QDateTime t = begin; t <= end; t = t.addSecs( 60 ) )
                    minor.push_back( datetime_to_usecs( t ) );
            }
            else if( d <= 60 * 60 * 1000 )
            {
                for( QDateTime t = begin; t <= end; t = t.addSecs( 10 * 60 ) )
                    minor.push_back( datetime_to_usecs( t ) );
            }
            else if( d <= 12 * 60 * 60 * 1000 )
            {
                for( QDateTime t = begin; t <= end; t = t.addSecs( 60 * 60 ) )
                {
                    minor.push_back( datetime_to_usecs( t ) );
                }
            }
            break;
        }
    }

    // sort and unique the major ticks
    qSort( major.begin(), major.end() );
    major.erase( std::unique( major.begin(), major.end() ), major.end() );

    // cut the minor ticks at the ends
    minor.erase( kd_lower_bound( minor.begin(), minor.end(), et, std::less< qint64 >() ), minor.end() );
    minor.erase( minor.begin(), kd_upper_bound( minor.begin(), minor.end(), st, std::less< qint64 >() ) );
}

QSize KDTimeLineWidget::sizeHint() const
{
  ensurePolished();
  const QFontMetrics fm = fontMetrics();
  return QSize( 100, 40 + fm.height() );
}

QSize KDTimeLineWidget::minimumSizeHint() const
{
  return QSize(20, sizeHint().height());
}

/*! If set to true, the current time indicator can only be positioned
  so it is aligned to minor tickmarks. If set to false, it can be positioned
  freely in time. */
void KDTimeLineWidget::setSnapToTickmarks(bool b)
{
  d->snaptoticks = b;
  setCurrentDateTime( d->snapTo( position(currentDateTime()) ) );
}

/*! \returns true if the current time indicator will snap to
  the minor tickmarks in this timeline and false otherwise.
  \see KDTimeLineWidget::setSnapToTickmarks(bool)
*/
bool KDTimeLineWidget::snapToTickmarks() const { return d->snaptoticks; }


QDateTime KDTimeLineWidget::Private::snapTo( int x )
{
    const QFontMetrics fm = q->fontMetrics();
    const int minw = fm.width( dateTimeToString( starttime) ) + fm.width( QLatin1String( "XXX" ) );

    QList< qint64 > major;
    QList< qint64 > minor;
    getTickmarkValues( major, minor, minw );
    const qint64 timex = viewToTime( x );

    if( minor.isEmpty() )
        return QDateTime();

    const QList< qint64 >::const_iterator bound = kd_lower_bound( minor.begin(), minor.end(), timex, std::less< qint64 >() );
    if( bound == minor.begin() )
        return usecs_to_datetime( *bound );
    else if( bound == minor.end() )
        return usecs_to_datetime( minor.last() );

    const qint64 lower = *(bound - 1);
    const qint64 upper = *bound;

    if( timex - lower < upper - timex )
        return usecs_to_datetime( lower );
    else
        return usecs_to_datetime( upper );
}

void KDTimeLineWidget::paintEvent( QPaintEvent* pe )
{
#if 0
  qDebug() << "KDTimeLineWidget::paintEvent() from " << d->starttime
           << " to " << d->endtime;
#endif
    const qint64 st = datetime_to_usecs( d->starttime );
    const qint64 et = datetime_to_usecs( d->endtime );
    if( st >= et )
        return;

    QStylePainter p( this );
    const QFontMetrics fm = p.fontMetrics();
    const int strh = fm.height();
    const int minw = fm.width( d->dateTimeToString(d->starttime) )+fm.width( QLatin1String( "XXX" ) );

    QList< qint64 > major;
    QList< qint64 > minor;
    d->getTickmarkValues( major, minor, minw );

    const qint64 starttime = st;
    const qint64 endtime = et;

    for( QList< qint64 >::const_iterator it = major.begin(); it != major.end(); ++it )
    {
        const QDateTime c = usecs_to_datetime( *it );
        const int x = d->timeToView( *it );
        const QString str = d->dateTimeToString( c );
        const int strw = fm.width( str );
        p.drawLine( x, strh + 3, x, height() );
        p.drawText( x - strw / 2, strh, str );
        minor.removeAll( *it );
    }
    p.setPen( palette().color( QPalette::Disabled, QPalette::WindowText ) );
    for( QList< qint64 >::const_iterator it = minor.begin(); it != minor.end(); ++it )
    {
        const QDateTime c = usecs_to_datetime( *it );
        const int x = d->timeToView( *it );
        p.drawLine( x, strh + 8, x, height() - 20 );
    }

  p.save();
  p.setPen( Qt::NoPen );
  p.setBrush( palette().dark() );
  p.drawRect( QRect( QPoint( pe->rect().left(), height()-20 ),
                     QPoint( pe->rect().right(), height() ) ) );
  p.restore();

  int elapsedx = d->timeToView(datetime_to_usecs(elapsedDateTime()));
  if( elapsedDateTime().isValid() && elapsedx > pe->rect().left() ) {
    p.save();
    p.setBrush( palette().base() );
    QStyleOption opt;
    opt.rect =  QRect( QPoint( pe->rect().left()-1, height()-20 ),
                       QPoint( elapsedx, height() ) );
    opt.palette = palette();
    opt.state = QStyle::State_Raised;
    p.drawPrimitive( QStyle::PE_PanelButtonBevel, opt );
    p.restore();
  }

  /* TimeLineWidget Items */
  QList<KDTimeLineWidgetItem*>::iterator lower = kd_upper_bound( d->items.begin(),
                                                             d->items.end(),
                                                             usecs_to_datetime(starttime),
                                                             KDTimeLineWidgetItemLessThan() );
  QList<KDTimeLineWidgetItem*>::iterator upper = kd_lower_bound( d->items.begin(),
                                                             d->items.end(),
                                                             usecs_to_datetime(endtime),
                                                             KDTimeLineWidgetItemLessThan() );
  for( QList<KDTimeLineWidgetItem*>::iterator it = lower; it != upper; ++it ) {
    KDTimeLineWidgetItem* item = *it;
    item->paintItem( &p, QPoint( d->timeToView(datetime_to_usecs(item->dateTime())), height()-20 ) );
  }

  /* Current Time Indicator */
  int curtimex = d->timeToView(datetime_to_usecs(currentDateTime()));
  if( currentDateTime().isValid() ) {
    p.save();
    p.setRenderHint( QPainter::Antialiasing );
    p.setBrush( palette().link() );
    QPolygon poly; /* TODO: Nicer drawing */
    poly.setPoints( 3,
                    -8,0,
                    8,0,
                    0,height()-20-strh );
    p.translate(curtimex,strh);
    p.drawConvexPolygon( poly );
    p.restore();
  }
}

/*! \fn void KDTimeLineWidget::itemSelected( KDTimeLineWidgetItem* )
  This signal is emitted when the user clicks an event item
 */
/*! \fn void KDTimeLineWidget::itemMoved( KDTimeLineWidgetItem* )
  This signal is emitted when the user has moved an event item
*/
/*! \fn void KDTimeLineWidget::currentDateTimeChanged(const QDateTime&)
  This signal is emitted when the user has changed the current datetime
*/

void KDTimeLineWidget::mousePressEvent( QMouseEvent* ev )
{
  if( ev->button() == Qt::LeftButton ) {
    if( snapToTickmarks() ) setCurrentDateTime( d->snapTo(ev->pos().x()));
    else setCurrentDateTime( usecs_to_datetime(d->viewToTime(ev->pos().x())) );
  }
}

void KDTimeLineWidget::mouseMoveEvent( QMouseEvent* ev )
{
  // TODO
  /* Pressing and dragging the timeline should scroll it */
  if( ev->buttons() & Qt::LeftButton ) {
    if( snapToTickmarks() ) setCurrentDateTime( d->snapTo(ev->pos().x()));
    else setCurrentDateTime( usecs_to_datetime(d->viewToTime(ev->pos().x())) );
  }
}

/***********************************************
 *               KDTimeLineArea
 **********************************************/



class KDTimeLineArea::Private {
    friend class ::KDTimeLineArea;
    KDTimeLineArea * const q;
public:
    Private( KDTimeLineArea * qq )
        : q( qq ),
          scrollbar( 0 ),
          viewport( 0 ),
          timeLineWidget( 0 )
    {

    }
    ~Private() {}

private:
    void updateScrollbar();

    void slotScrollValueChanged( int );
    void slotZoomValueChanged( int );

private:
  KDTimeScrollBar* scrollbar;
  QWidget* viewport;
  KDTimeLineWidget* timeLineWidget;
};

/* TODO(steffen): Review.
   I could not get things to work without introducing
   a constant resolution SCROLL_RANGE. The problem is that
   without this, the resulution is determined "dynamically"
   by the width of the timeline. This means that zooming
   leads to a change in resolution and that made zooming
   jumpy and erratic.
*/
static const int SCROLL_RANGE = 2000;

/*! \class KDTimeLineArea KDTimeLineArea
  \ingroup KDTimeLineWidget
  \brief This class encapsulates a KDTimeLineWidget and a KDTimeScrollBar
  into a single widget.

  Scrolling and zooming the scrollbar influences the timeline
*/

/*! Constructor. Creates a KDTimeLineArea with parent
  \a parent and with a KDTimeLineWidget instance in it. */
KDTimeLineArea::KDTimeLineArea( QWidget * parent )
  : QFrame( parent ), d( new Private( this ) )
{
  setFrameStyle( QFrame::Panel|QFrame::Sunken );
  d->scrollbar = new KDTimeScrollBar;
  d->viewport = new QWidget;
  d->viewport->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Minimum );
  setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Fixed );

  QVBoxLayout* toplayout = new QVBoxLayout(this);
  toplayout->setMargin( frameWidth()/2 );
  toplayout->setSpacing(0);
  toplayout->addWidget(d->scrollbar);
  toplayout->addWidget(d->viewport);
  setLayout(toplayout);
  setTimeLineWidget( new KDTimeLineWidget );

  d->viewport->installEventFilter(this);
  d->scrollbar->installEventFilter(this);
  d->scrollbar->setRange( 0, SCROLL_RANGE );

  connect( d->scrollbar, SIGNAL(valueChanged(int)),
           this, SLOT(slotScrollValueChanged(int)));
  connect( d->scrollbar, SIGNAL(zoomChanged(int)),
           this, SLOT(slotZoomValueChanged(int)));
}

KDTimeLineArea::~KDTimeLineArea() {}


/*! Provided for users who want to subclass
  KDTimeLineWidget and supply their custom subclass
  for use in a KDTimeLineArea. KDTimeLineArea takes ownership of
  of the timeline. The previously set timeline is deleted.
*/
void KDTimeLineArea::setTimeLineWidget( KDTimeLineWidget* tl )
{
  Q_ASSERT(tl);
  if( tl == d->timeLineWidget ) return;
  delete d->timeLineWidget;
  tl->setParent(d->viewport);
  d->timeLineWidget = tl;
  d->viewport->setMinimumSize(d->timeLineWidget->minimumSizeHint());
  connect( d->timeLineWidget, SIGNAL(currentDateTimeChanged(QDateTime)),
           this, SLOT(ensureVisible(QDateTime)));
}

/*! \returns a pointer to the KDTimeLineWidget used by this KDTimeLineArea */
KDTimeLineWidget* KDTimeLineArea::timeLineWidget() const { return d->timeLineWidget; }

/*! Sets the start time of the timeline to \a dt.
  Don't call KDTimeLineWidget::setStartDateTime() directly,
  call this method instead.
*/
void KDTimeLineArea::setStartDateTime( const QDateTime& dt )
{
  d->timeLineWidget->setStartDateTime(dt);
  d->updateScrollbar();
}

/*! Sets the end time of the timeline to \a dt.
  Don't call KDTimeLineWidget::setEndDateTime() directly,
  call this method instead.
*/
void KDTimeLineArea::setEndDateTime( const QDateTime& dt )
{
  d->timeLineWidget->setEndDateTime(dt);
  d->updateScrollbar();
}

/*! Set the value of the scrollbar to \a v.
 */
void KDTimeLineArea::setScrollValue(int v)
{
  d->scrollbar->setValue(v);
}

/*! \returns the zoom value of the scrollbar. */
int KDTimeLineArea::zoomValue() const
{
  return d->scrollbar->zoomValue();
}

/*! Set the zoom value of the scrollbar to \a v.
 */
void KDTimeLineArea::setZoomValue(int v)
{
  d->scrollbar->setZoomValue(v);
}

/* TODO */
void KDTimeLineArea::displayDateTime( const QDateTime& start, const QDateTime& end )
{
  /* TODO */
    Q_UNUSED( start );
    Q_UNUSED( end );
    d->updateScrollbar();
}

/*!\returns a pointer to the KDTimeScrollBar used by this KDTimeLineArea */
KDTimeScrollBar* KDTimeLineArea::scrollBar() const { return d->scrollbar; }


/*! Set the current datetime displayed on the
  timeline to \a dt.
  \see KDTimeLineWidget::setCurrentDateTime(const QDateTime&)
 */
void KDTimeLineArea::setCurrentDateTime( const QDateTime& dt)
{
  d->timeLineWidget->setCurrentDateTime(dt);
  ensureVisible(dt);
}

/*! \returns the current datetime of the timeline */
QDateTime KDTimeLineArea::currentDateTime() { return d->timeLineWidget->currentDateTime(); }

/*! Set the elapsed datetime displayed on the
  timeline to \a dt.
  \see KDTimeLineWidget::setElapsedDateTime(const QDateTime&)
 */
void KDTimeLineArea::setElapsedDateTime( const QDateTime& dt)
{
  d->timeLineWidget->setElapsedDateTime(dt);
}

/*! Scrolls the timeline so the point \a dt in time
  becomes visible.
*/
void KDTimeLineArea::ensureVisible( const QDateTime& dt )
{
  int pos = d->timeLineWidget->position(dt);
  int v;
  if( pos < -d->timeLineWidget->x()+10 ) {
    v = pos-10;
  } else if( pos > -d->timeLineWidget->x()+d->viewport->width()-10 ) {
    v = pos-d->viewport->width()+10;
  } else {
    return;
  }
  if( v < 0 ) v = 0;
  else if( v > d->timeLineWidget->width()-d->viewport->width() ) v = d->timeLineWidget->width()-d->viewport->width();
  d->timeLineWidget->move( -v, 0 );
  d->updateScrollbar();
}

/*! Sets the previous item on the timeline to be current and makes
  sure it is visible. Does nothing if there is no previous item. */
void KDTimeLineArea::previousItem()
{
  timeLineWidget()->previousCurrentItem();
  KDTimeLineWidgetItem* item = timeLineWidget()->currentItem();
  if( item ) ensureVisible( item->dateTime() );
}

/*! Sets the next item on the timeline to be current and makes
  sure it is visible. Does nothing if there is no next item. */
void KDTimeLineArea::nextItem()
{
  timeLineWidget()->nextCurrentItem();
  KDTimeLineWidgetItem* item = timeLineWidget()->currentItem();
  if( item ) ensureVisible( item->dateTime() );
}

void KDTimeLineArea::Private::updateScrollbar()
{
    const int v = -timeLineWidget->x()*SCROLL_RANGE/timeLineWidget->width();
    const int z = viewport->width()*SCROLL_RANGE/timeLineWidget->width();
    //qDebug() << "KDTimeLineWidget::updateScrollbar(), x="<<timeLineWidget->x()<<", w="<<timeLineWidget->width()<<", v = " << v << ", z = " << z;
    if ( v < 0 )
        return;
    //Q_ASSERT(v >= 0);
    scrollbar->setValue( v );
    scrollbar->setZoomValue( z );
}

void KDTimeLineArea::Private::slotScrollValueChanged( int v )
{
    Q_ASSERT( v >= 0 );
    timeLineWidget->move( -v * timeLineWidget->width() / SCROLL_RANGE, 0 );
}

void KDTimeLineArea::Private::slotZoomValueChanged( int v )
{
    timeLineWidget->resize( SCROLL_RANGE * viewport->width() / v,
                            viewport->height() );
    slotScrollValueChanged( scrollbar->value() );
}

bool KDTimeLineArea::eventFilter( QObject* ob, QEvent* ev )
{
  if( ob == d->viewport ) {
    if( ev->type() == QEvent::Resize ) {
      d->timeLineWidget->resize( d->timeLineWidget->size().expandedTo(d->viewport->size()).width(), d->viewport->height() );
      d->slotZoomValueChanged(d->scrollbar->zoomValue());
    } else if( ev->type() == QEvent::KeyPress ) {
      QKeyEvent* kev = static_cast<QKeyEvent*>(ev);
      int z;
      switch( kev->key() ) {
      case Qt::Key_Plus:
        z = d->scrollbar->zoomValue()*2;
        if( z < d->scrollbar->minimumZoom() ) z = d->scrollbar->minimumZoom();
        else if( z > d->scrollbar->maximumZoom() ) z = d->scrollbar->maximumZoom();
        d->scrollbar->setZoomValue( z ); return true;
      case Qt::Key_Minus:
        z = d->scrollbar->zoomValue()/2;
        if( z < d->scrollbar->minimumZoom() ) z = d->scrollbar->minimumZoom();
        else if( z > d->scrollbar->maximumZoom() ) z = d->scrollbar->maximumZoom();
        d->scrollbar->setZoomValue( z ); return true;
      case Qt::Key_Left:
        /*TODO*/;
      case Qt::Key_Right:
        /*TODO*/;
      }
    }
  } else if( ob == d->scrollbar && ev->type() == QEvent::Resize ) {
    emit zoomRangeChanged( d->scrollbar->minimumZoom(), d->scrollbar->maximumZoom() );
  }
  return false;
}

void KDTimeLineArea::keyPressEvent(QKeyEvent* ev)
{
  switch( ev->key() ) {
  case Qt::Key_Right:
    d->timeLineWidget->nextCurrentDateTime();
    ensureVisible( d->timeLineWidget->currentDateTime() );
    break;
  case Qt::Key_Left:
    d->timeLineWidget->previousCurrentDateTime();
    ensureVisible( d->timeLineWidget->currentDateTime() );
    break;
  }
}

#include "moc_kdtimelinewidget.cpp"
