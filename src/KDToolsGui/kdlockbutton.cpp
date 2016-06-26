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

#include "kdlockbutton.h"

#include "macwidget_p.h"

#include <QFile>
#include <QPainter>
#include <QPixmapCache>
#include <QTimeLine>
#include <QImageReader>

using namespace _macwidget;

static const char NATIVE_MAC_PATH[] = "/System/Library/Frameworks/SecurityInterface.framework/Resources/";

/*!
 \internal
 */
class KDLockButton::Private
{
    friend class ::KDLockButton;
    KDLockButton* const q;
public:
    explicit Private( KDLockButton * qq )
        : q( qq ),
          autoLock( false )
    {
#ifdef Q_OS_MAC
        animation.setFrameRange( 0, 16 );
        animation.setDuration( 500 );
        connect( &animation, SIGNAL(frameChanged(int)), q, SLOT(update()));
#endif
    }

private:
    /*!
     Emits lockChanged.
     \internal
    */
    void _kdt_checkChanged( bool checked )
    {
        emit q->lockChanged( !checked );
#ifdef Q_OS_MAC
        animation.setDirection( checked ? QTimeLine::Forward : QTimeLine::Backward );
        q->update();
        animation.start();
#endif
    }

    QString macExtension() const
    {
#ifdef Q_OS_MAC
        if ( QSysInfo::MacintoshVersion > QSysInfo::MV_SNOWLEOPARD )
            return QLatin1String("png");
        else
            return QLatin1String("tif");
#else
        return QString();
#endif
    }

    bool useNativeMacPixmaps() const
    {
#ifdef Q_OS_MAC
       if(!isMacStyle( q->style() ) )
          return false;
       static bool use = ( QSysInfo::MacintoshVersion > QSysInfo::MV_SNOWLEOPARD ) ? QImageReader::supportedImageFormats().contains( QByteArray( "png" ) )
                                                                                   : QImageReader::supportedImageFormats().contains( QByteArray( "tiff" ) );
       return use;
#else
       return false;
#endif
    }

#if QT_VERSION >= 0x040600
    static bool findPixmap( const QPixmapCache::Key & key, QPixmap * pix ) {
        return QPixmapCache::find( key, pix );
    }
    static QPixmapCache::Key insertPixmap( const QPixmap & pix ) {
        return QPixmapCache::insert( pix );
    }
#else
    static bool findPixmap( const QString & key, QPixmap * pix ) {
        const QPixmap * pixmap = QPixmapCache::find( key );
        if ( pixmap )
            *pix = *pixmap;
        return pixmap;
    }
    static QString insertPixmap( const QPixmap & pix ) {
        const QString key = QString::number( pix.cacheKey() );
        QPixmapCache::insert( key, pix );
        return key;
    }
#endif


    QPixmap pixmap() const {
#ifdef Q_OS_MAC
        if ( useNativeMacPixmaps() && animation.state() == QTimeLine::Running ) {
            int animationValue = animation.currentFrame();
            if ( animationValue == 0 )
                animationValue = 1;
            else if ( animationValue == 16 )
                animationValue = 15;

            QPixmap pixmap;
            if ( !findPixmap( anim[animationValue], &pixmap ) ) {
                pixmap.load( QLatin1String( NATIVE_MAC_PATH ) + QString::fromLatin1( "Lock_Open Anim %1.%2" ).arg( animationValue, 2, 10, QLatin1Char( '0' ) ).arg( macExtension() ) );
                anim[animationValue] = insertPixmap( pixmap );
            }
            return pixmap;
        }
#endif
        return pixmap( q->isLocked() );
    }

    QPixmap pixmap( bool locked ) const {
        QPixmap pix;
        if ( !findPixmap( key[locked], &pix ) ) {
            pix.load( useNativeMacPixmaps() ? QString::fromLatin1( NATIVE_MAC_PATH ) + QString::fromLatin1( locked ? "Lock_Locked State.%1"
                                                                                                                   : "Lock_Unlocked State.%1" ).arg( macExtension() )
                                            : RESOURCE( locked ? "lockbutton_lock_locked"
                                                               : "lockbutton_lock_unlocked" ) );
            key[locked] = insertPixmap( pix );
        }
        return pix;
    }

    QPixmap lockedPixmap() const {
        return pixmap( true );
    }

    QPixmap unlockedPixmap() const {
        return pixmap( false );
    }

private:
#if QT_VERSION >= 0x040600
    mutable QPixmapCache::Key key[2], anim[16];
#else
    mutable QString key[2], anim[16];
#endif
    QString lockedText;
    bool autoLock;
#ifdef Q_OS_MAC
    QTimeLine animation;
#endif
};

/*!
 \class KDLockButton
 \ingroup gui macwidgets
 \brief A Mac-style widget to indicate GUI elements are locked from/by the user.

 A KDLockButton can be used in dialogs which contain user elements locked from the user. The user can unlock them with KDLockButton.
 KDLockButton can either automatically lock/unlock itself upon click or just notify the program via the signals lockRequested() and unlockRequested() about the user's
 wish, which can grant or deny the request by calling the slots lock() or #unlock().

 KDLockButton has a text property and a lockedText property. The text is shown when the button is unlocked. It defaults to "Click the lock to prevent further changes."
 The lockedText is shown when the button is locked. It defaults to "Click the lock to make changes."
*/

/*!
 \fn KDLockButton::lockRequested

 Emitted whenever when autoLock is off and the user tries to lock the button.
*/

/*!
 \fn KDLockButton::unlockRequested

 Emitted whenever when autoLock is off and the user tries to unlock the button.
*/

/*!
 \fn KDLockButton::lockChanged( bool locked )

 Emitted whenever the lock state of the button changed to \a locked.
*/



/*!
 Creates a new KDLockButton with \a parent
 */
KDLockButton::KDLockButton( QWidget* parent )
    : QAbstractButton( parent ),
      d( new Private( this ) )
{
    setAttribute( Qt::WA_Hover );
    setCheckable( true );
    connect( this, SIGNAL(toggled(bool)),
             this, SLOT(_kdt_checkChanged(bool)) );

    setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );

    setText( tr( "Click the lock to prevent further changes." ) );
    setLockedText( tr( "Click the lock to make changes." ) );
}

/*!
 Destroys the KDLockButton.
 */
KDLockButton::~KDLockButton()
{
}

/*!
 \property KDLockButton::lockedText

 Specifies the text shown when the button is locked. The default value is "Click the lock to make changes."

 Get this property's value using %lockedText(), and set it using %setLockedText().
 */

void KDLockButton::setLockedText( const QString& lockedText )
{
    if ( lockedText == d->lockedText )
        return;
    d->lockedText = lockedText;
    update();
}

QString KDLockButton::lockedText() const
{
    return d->lockedText;
}

/*!
 Locks the button. This is the same as calling setLocked(true).
*/
void KDLockButton::lock()
{
    setLocked( true );
}

/*!
 Unlocks the button. This is the same as calling setLocked(false).
*/
void KDLockButton::unlock()
{
    setLocked( false );
}

/*!
 \property KDLockButton::locked

 Specifies whether the button is locked or not. This is the opposite of QAbstractButton's checked and is provided to avoid confustion.
 Per default the button is locked.

 Get this property's value using %isLocked(), and set it using %setLocked().
*/

void KDLockButton::setLocked( bool locked )
{
    setChecked( !locked );
}

bool KDLockButton::isLocked() const
{
    return !isChecked();
}

/*!
  \property KDLockButton::autoLock

  Specifies, whether the lock button is locking/unlocking itself
  (autoLock = true) or is just emitting the signals lockRequested and
  unlockRequested when the user clicks them (autoLock = false). The
  default value is false.

  Get this property's value using %autoLock(), and set it using %setAutoLock().
*/

void KDLockButton::setAutoLock( bool autoLock )
{
    d->autoLock = autoLock;
}

bool KDLockButton::autoLock() const
{
    return d->autoLock;
}

void KDLockButton::nextCheckState()
{
    if( d->autoLock )
        QAbstractButton::nextCheckState();
    else if( isChecked() )
        emit lockRequested();
    else
        emit unlockRequested();
}

static const int SPACING = 5;

QSize KDLockButton::minimumSizeHint() const
{
    return sizeHint();
}

QSize KDLockButton::sizeHint() const
{
    const QFontMetrics fm = fontMetrics();
    const int textWidth = qMax( fm.width( text() ), fm.width( lockedText() ) );
    const QRect iconRect = QRect( QPoint(), d->lockedPixmap().size().expandedTo( d->unlockedPixmap().size() ) );
    return QSize( iconRect.width() + SPACING + textWidth, iconRect.height() );
}

void KDLockButton::paintEvent( QPaintEvent * )
{
    QPainter painter( this );
    const QRectF iconRect = QRect( QPoint(), d->lockedPixmap().size().expandedTo( d->unlockedPixmap().size() ) );
    const QPixmap pix = d->pixmap();
    QRectF realIconRect = pix.rect();
    realIconRect.moveCenter( iconRect.center() );
    if( underMouse() )
    {
        const QRectF r = QRectF( iconRect ).adjusted( 0.5, 0.5, -0.75, -0.75 );
        const QPen pen = painter.pen();
        painter.setBrush( QColor( 216, 216, 216 ) );
        painter.setPen( QColor( 194, 194, 194 ) );
        painter.drawRoundedRect( r, 1.5, 1.5 );
        painter.setPen( pen );
    }
    painter.drawPixmap( realIconRect.topLeft(), pix );

    const QRectF textRect( QPointF( iconRect.right() + SPACING, iconRect.top() ), rect().bottomRight() + QPointF(1.0, 1.0) );
    painter.drawText( textRect, Qt::AlignBottom, isChecked() ? text() : d->lockedText );
}

#include "moc_kdlockbutton.cpp"
