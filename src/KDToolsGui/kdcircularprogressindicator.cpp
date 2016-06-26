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

#include "kdcircularprogressindicator.h"

#include <QPainter>
#include <QBasicTimer>
#include <QTimerEvent>

static const unsigned int numAnimationSteps = 12;
static const int animationStepMS = 500 / numAnimationSteps; // 2rpm

/*!
 \internal
 */
class KDCircularProgressIndicator::Private
{
public:
    explicit Private( KDCircularProgressIndicator* q )
        : q( q ),
          animationStep( 0 )
    {
    }

private:
    KDCircularProgressIndicator* const q;

public:
    unsigned int animationStep;
    QBasicTimer timer;
};

/*!
 \class KDCircularProgressIndicator KDCircularProgressIndicator
 \ingroup gui macwidgets
 \brief A Mac-style circular (rotating) progress indicator

 A KDCircularProgressIndicator is usually used on Mac OS to inform the
 user about a process currently running in background. It doesn't really
 show progress like a QProgressBar.
 */

/*!
 Creates a new KDCircularProgressIndicator with parent \a parent.
 */
KDCircularProgressIndicator::KDCircularProgressIndicator( QWidget* parent )
    : QWidget( parent ),
      d( new Private( this ) )
{
    setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    setActive( true );

    QPalette p;
    p.setColor( QPalette::WindowText, QColor( 78, 78, 78 ) );
    setPalette( p );
}

/*!
 Destroys the KDCircularProgressIndicator
 */
KDCircularProgressIndicator::~KDCircularProgressIndicator()
{
}

/*!
 \reimp
 */
int KDCircularProgressIndicator::heightForWidth( int w ) const
{
   // we would like to be square
   return w;
}

/*!
 \reimp
 */
QSize KDCircularProgressIndicator::minimumSizeHint() const
{
   return sizeHint();
}

/*!
 \reimp
 */
QSize KDCircularProgressIndicator::sizeHint() const
{
   return QSize( 17, 17 );
}

/*!
 \property KDCircularProgressIndicator::active

 Specifies whether the circular progress indicator is active or not. Even if the
 progress indicator is not active, it fills space in a layout.

 Get this property's value using %isActive(), and set it using %setActive().
*/

bool KDCircularProgressIndicator::isActive() const
{
    return d->timer.isActive();
}

/*!
  See #active.
*/
void KDCircularProgressIndicator::setActive( bool active )
{
    if ( active && !d->timer.isActive() ) {
        d->timer.start( animationStepMS, this );
        update();
    } else if ( !active && d->timer.isActive() ) {
        d->timer.stop();
        update();
    }
}

/*!
 \reimp
 */
void KDCircularProgressIndicator::timerEvent( QTimerEvent * e )
{
    if ( e->timerId() == d->timer.timerId() ) {
        d->animationStep += 1;
        d->animationStep %= numAnimationSteps;
        update();
    } else {
        QWidget::timerEvent( e );
    }
}

/*!
 Create a mix of colors.
 \internal
 */
static QColor mixColor( const QColor& color1, qreal amount1, const QColor& color2, qreal amount2 = -1.0 )
{
    if( amount2 == -1.0 )
        amount2 = 1.0 - amount1;
    qreal r1 = color1.red() * amount1;
    qreal g1 = color1.green() * amount1;
    qreal b1 = color1.blue() * amount1;
    qreal a1 = color1.alpha() * amount1;
    const qreal r2 = color2.red() * amount2;
    const qreal g2 = color2.green() * amount2;
    const qreal b2 = color2.blue() * amount2;
    const qreal a2 = color2.alpha() * amount2;

    r1 += r2;
    g1 += g2;
    b1 += b2;
    a1 += a2;

    return QColor( static_cast<int>(r1) % 256,
                   static_cast<int>(g1) % 256,
                   static_cast<int>(b1) % 256,
                   static_cast<int>(a1) % 256);
}

/*!
 \reimp
 */
void KDCircularProgressIndicator::paintEvent( QPaintEvent * )
{
    if( !isActive() )
        return;

    QPainter p( this );

    const int size = qMin( height(), width() );

    const QRectF bar( size / -17.0, size / (-17.0/7.0) - 1.0, size/17.0 * 1.75, size/17.0*4.5 );

    p.setRenderHint( QPainter::Antialiasing, true );
    p.translate( rect().center() );

    p.rotate( d->animationStep * 360.0 / numAnimationSteps );

    p.setPen( Qt::transparent );
    p.setBrush( palette().color( QPalette::Foreground ) );

    const QColor foreground = palette().color( QPalette::WindowText );

    for( unsigned int i = 0; i < numAnimationSteps; ++i )
    {
        p.setBrush( mixColor( Qt::transparent, i / static_cast<qreal>(numAnimationSteps+1), foreground ) );
        p.drawEllipse( bar );
        p.rotate( -360.0 / numAnimationSteps );
    }
}

#include "moc_kdcircularprogressindicator.cpp"
