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

#include "kdtimescrollbar.h"

#include <QApplication>
#include <QMouseEvent>
#include <QPainterPath>
#include <QStyleOptionSlider>
#include <QStylePainter>

/*!
  \class KDTimeScrollBar KDTimeScrollBar
  \ingroup KDTimeLineWidget

  \brief This widget is a scrollbar-like widget that has
  the additional property that the user can resize
  the elevator part of the scrollbar. The size of the
  elevator can be used as a zoom-control.

  \note Currently this widget only works in Qt::Horizontal mode
*/

class KDTimeScrollBar::Private {
    friend class ::KDTimeScrollBar;
    KDTimeScrollBar * const q;
public:
    Private( KDTimeScrollBar * qq );
    ~Private();


private:
    void clampValue();
    void getViewValues( int* x1, int* x2 );
    int valueFromView(int) const;

private:
    int minimum;
    int maximum;
    int value;
    int zoom;

    int delta;
    int moffset;
    int voffset;
    bool scrolling;
    bool zooming;
};

KDTimeScrollBar::Private::Private( KDTimeScrollBar * qq )
    : q( qq ),
      minimum( 0 ),
      maximum( 1000 ),
      value( 0 ),
      zoom( 333 ),
      scrolling( false ),
      zooming( false )
{
    q->setFocusPolicy( Qt::StrongFocus );
    q->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Fixed );
}

KDTimeScrollBar::Private::~Private() {}


/*!
  Constructor. Creates a KDTimeScrollBar with parent \a parent,
  minimum value 0, maximum value 100, minimum zoom 1, maximum zoom 10,
  value 10 and zoom value 5
*/
KDTimeScrollBar::KDTimeScrollBar( QWidget * parent_ )
    : QWidget( parent_ ), d( new Private( this ) )
{

}

KDTimeScrollBar::~KDTimeScrollBar() {}

/*! Reimplemented from QWidget */
QSize KDTimeScrollBar::sizeHint() const
{
  /* More of less lifted from QScrollBar */
  ensurePolished();
  QStyleOptionSlider opt;
  opt.init(this);
  opt.subControls = QStyle::SC_None;
  opt.activeSubControls = QStyle::SC_None;
  opt.orientation = Qt::Horizontal;
  opt.state |= QStyle::State_Horizontal;

  int scrollBarExtent = style()->pixelMetric(QStyle::PM_ScrollBarExtent, &opt, this);
  int scrollBarSliderMin = style()->pixelMetric(QStyle::PM_ScrollBarSliderMin, &opt, this);
  QSize sz;
  if (opt.orientation == Qt::Horizontal)
    sz = QSize(scrollBarExtent * 2 + scrollBarSliderMin, scrollBarExtent);
  else
    sz = QSize(scrollBarExtent, scrollBarExtent * 2 + scrollBarSliderMin);

  return style()->sizeFromContents(QStyle::CT_ScrollBar, &opt, sz, this)
    .expandedTo(QApplication::globalStrut());
}

/*! Set the allowed range of the scrollbar to [\a min, \a max].
  The actual values that the user can scroll the scrollbar to are
  between \a min+zoomValue()/2 and \a max-zoomValue()/2 */
void KDTimeScrollBar::setRange( int min, int max)
{
#if 0
  qDebug() << "KDTimeScrollBar::setRange( " << min << ", " << max << " )";
  qDebug() << "               oldrange is " << d->minimum << ", " << d->maximum;
#endif
  Q_ASSERT(min<max);
  Q_ASSERT(d->minimum<d->maximum);

  //  qDebug() << "Changing values from d->value="<<d->value<<", d->zoom="<<d->zoom;
  d->zoom = (d->zoom*(max-min))/(d->maximum-d->minimum);
  d->value = (d->value*(max-min))/(d->maximum-d->minimum);
  //qDebug() << "                  to d->value="<<d->value<<", d->zoom="<<d->zoom;

  d->minimum = min;
  d->maximum = max;
  d->clampValue();
  update();
}

void KDTimeScrollBar::setMinimumValue( int min )
{
    setRange( min, d->maximum );
}

void KDTimeScrollBar::setMaximumValue( int max )
{
    setRange( d->minimum, max );
}

int KDTimeScrollBar::minimumZoom() const
{
  return QStyle::sliderValueFromPosition( d->minimum, d->maximum, 3*height(), width() );
}
int KDTimeScrollBar::maximumZoom() const
{
  return d->maximum-d->minimum;
}

/*! Set the value of the scrollbar. If the supplied value is
  outside the allowed range, it will be set to the nearest allowed
  value. If the value differs from the current value, a signal is
  emitted */
void KDTimeScrollBar::setValue(int v)
{
  int old_zoom = d->zoom;
  int old_value = d->value;
  d->value = v;
  d->clampValue();
  if( old_zoom != d->zoom ) {
    //qDebug() << "emit zoomChanged("<<d->zoom<<")";
    emit zoomChanged(d->zoom);
    repaint(); // Using update() made the bar less smooth
  }
  if( old_value != d->value ) {
    //qDebug() << "emit valueChanged("<<d->value<<")";
    emit valueChanged(d->value);
    repaint(); // Using update() made the bar less smooth
  }
}

int KDTimeScrollBar::minimumValue() const
{ return d->minimum; }
int KDTimeScrollBar::maximumValue() const
{ return d->maximum; }
int KDTimeScrollBar::value() const
{ return d->value; }
int KDTimeScrollBar::zoomValue() const
{ return d->zoom; }

/*! Set the value of the zoom bar. If the supplied value is
  outside the allowed range, it will be set to the nearest allowed
  value. If the value differs from the current value, a signal is
  emitted */
void KDTimeScrollBar::setZoomValue(int zoom)
{
  if( d->maximum-d->minimum < zoom ) { // PENDING(hansen) review
    zoom = d->maximum-d->minimum;
  }
  int old_zoom = d->zoom;
  int old_value = d->value;
  d->zoom = zoom;
  d->clampValue();
  if( old_zoom != d->zoom ) {
    //qDebug() << "emit zoomChanged("<<d->zoom<<")";
    emit zoomChanged(d->zoom);
    repaint();
  }
  if( old_value != d->value ) {
    //qDebug() << "emit valueChanged("<<d->value<<")";
    emit valueChanged(d->value);
    repaint();
  }
  //qDebug() << "KDTimeScrollBar::setZoomValue( " << zoom << " ), d->value="<<d->value<<", d->zoom="<<d->zoom;
}


/*! \fn void KDTimeScrollBar::valueChanged(int)
  This signal is emitted when the scrollbar is
  scrolled to a new value.
 */
/*! \fn void KDTimeScrollBar::zoomChanged(int)
  This signal is emitted when the scrollbar is
  zoomed to a new value.
 */


void KDTimeScrollBar::Private::clampValue()
{
    value = qBound( minimum, value, maximum - zoom );
    Q_ASSERT(value>=minimum);
    Q_ASSERT(value<=maximum);
  
    if( zooming ) 
    {
        const int z = QStyle::sliderPositionFromValue( minimum, maximum, zoom, q->width() );
        if( z < 2.25 * q->height() )
        {
            zoom = QStyle::sliderValueFromPosition( minimum, maximum, q->height() * 9/4, q->width() );
            value = qBound( minimum, value, maximum - zoom );
        }

#if 0
    if ( zoom < (maximum-minimum)*q->height()/q->width() )
      zoom = (maximum-minimum)*q->height()/q->width(); /* Why do we need this? */
    if ( zoom > maximum-minimum )
      zoom = maximum - minimum;
#endif
    }
    Q_ASSERT(zoom>0);
    Q_ASSERT(zoom<=maximum-minimum);
}

void KDTimeScrollBar::Private::getViewValues( int* x1, int* x2 )
{
  *x1 = QStyle::sliderPositionFromValue( minimum, maximum, value,      q->width() );
  *x2 = QStyle::sliderPositionFromValue( minimum, maximum, value+zoom, q->width() );
}

int KDTimeScrollBar::Private::valueFromView(int x) const
{
  //return x*(maximum-minimum)/(q->width()/*-q->height()*/) + minimum;
  return QStyle::sliderValueFromPosition( minimum, maximum, x, q->width() );
}

void KDTimeScrollBar::resizeEvent( QResizeEvent* )
{
  d->zooming = d->scrolling = true;
  d->clampValue();
  d->zooming = d->scrolling = false;
}


void KDTimeScrollBar::paintEvent( QPaintEvent* ev )
{
  Q_UNUSED(ev);
  QStylePainter p(this);
  int xmin,xmax;
  d->getViewValues(&xmin,&xmax);
  //qDebug() << "paintEvent(): xmin = " << xmin << ", xmax = " << xmax << ", x = " << x;

  // "knob"
  // PENDING(kalle) Use pixelMetric() for line width
  const QPointF tl( xmin + height(),     height() / 3.5 );
  const QPointF br( xmax - height() - 1, height() * 2.5 / 3.5 );
  QLinearGradient gradient( tl, br );
  gradient.setColorAt( d->scrolling ? 1.0 : 0.0, palette().color( QPalette::Dark ) );
  gradient.setColorAt( d->scrolling ? 0.0 : 1.0, palette().color( QPalette::Mid ) );
  // QRectF(QPointF,QPointF) wasn't in 4.2/4.1:
  // PENDING(marc) port to KDRectF, when available
  const QRectF r( tl.x(), tl.y(), br.x() - tl.x(), br.y() - tl.y() );
  p.fillRect( r, gradient );
  
  gradient.setColorAt( d->zooming ? 1.0 : 0.0, palette().color( QPalette::Dark ) );
  gradient.setColorAt( d->zooming ? 0.0 : 1.0, palette().color( QPalette::Mid ) );

  // left subhandle
  // PENDING(kalle) Use pixelMetric() for line width
  const QRect rLeft( xmin, 0, height(), height() );
  QPainterPath path;
  path.moveTo( rLeft.topRight() );
  path.lineTo( rLeft.bottomRight() );
  path.lineTo( rLeft.left(), rLeft.center().y() );
  p.fillPath( path, gradient );

  const QRect rRight( xmax - height() + 1, 0, height(), height() );
  path = QPainterPath();
  path.moveTo( rRight.topLeft() );
  path.lineTo( rRight.bottomLeft() );
  path.lineTo( rRight.right(), rLeft.center().y() );
  p.fillPath( path, gradient );
}

void KDTimeScrollBar::mousePressEvent( QMouseEvent* ev )
{
  //qDebug() << "KDTimeScrollBar::mousePressEvent( " << ev << " )";
  if( ev->buttons() & Qt::LeftButton ) {
    int xmin,xmax;
    d->getViewValues(&xmin,&xmax);
    if( ev->pos().x() < xmin+height() && ev->pos().x() >= xmin ) {
      d->zooming = true;
      d->scrolling = false;
      d->delta = ev->pos().x()-xmin;
    } else if( ev->pos().x() > xmax-height() && ev->pos().x() <= xmax ) {
      d->zooming = true;
      d->scrolling = false;
      d->delta = xmax-ev->pos().x();
    } else if( ev->pos().x() > xmin+height() && ev->pos().x() < xmax-height() ) {
      d->zooming = false;
      d->scrolling = true;
      //d->delta = ev->pos().x()-(xmin+(xmax-xmin)/2);
      d->delta = ev->pos().x()-xmin;
      d->moffset = ev->pos().x();
      d->voffset = xmin;
    } else {
      // Jump
      d->zooming = false;
      d->scrolling = true;
      d->delta = 0;
      setValue( d->valueFromView(ev->pos().x())-d->zoom/2 );
      d->moffset = ev->pos().x();
      d->voffset = xmin;
      emit valueChanged(d->value);
    }
  }
  update();
}

void KDTimeScrollBar::mouseReleaseEvent( QMouseEvent* ev )
{
  Q_UNUSED(ev);
  d->delta = 0;
  d->zooming = false;
  d->scrolling = false;
  update();
}

void KDTimeScrollBar::mouseMoveEvent( QMouseEvent* ev )
{
  //qDebug() << "KDTimeScrollBar::mouseMoveEvent( " << ev << " )";
  if( ev->buttons() & Qt::LeftButton ) {
    int xmin,xmax;
    d->getViewValues(&xmin,&xmax);
    int x = xmin+(xmax-xmin)/2;
    int pos = ev->pos().x()-d->delta;
    if( d->scrolling ) {
      setValue( d->valueFromView( d->voffset + ( ev->pos().x() - d->moffset ) ) );
    } else if( d->zooming ) {
      if( pos<x ) {
	int z = d->valueFromView( 2*(x-pos) );
	if( z >= minimumZoom() ) {
	  setValue( value()+(zoomValue()-z)/2 );
	  setZoomValue( z );
	}
      } else {
	int z = d->valueFromView( (pos+2*d->delta-xmin) );
	if( z >= minimumZoom() ) {
	  setValue( value()+(zoomValue()-z)/2 );
	  setZoomValue( z );
	}
      }
    }
  }
}

void KDTimeScrollBar::keyPressEvent( QKeyEvent* ev )
{
  switch( ev->key() ) {
  case Qt::Key_Plus:
    setZoomValue( zoomValue()*2 );
  case Qt::Key_Minus:
    setZoomValue( zoomValue()/2 );
  case Qt::Key_Left:
    setValue( value()+1 );
  case Qt::Key_Right:
    setValue( value()-1 );
  }
}

#include "moc_kdtimescrollbar.cpp"
