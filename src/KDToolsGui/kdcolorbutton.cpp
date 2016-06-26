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

#include "kdcolorbutton.h"

#include <QColor>
#include <QAction>
#include <QColorDialog>
#include <QPainter>
#include <QPixmap>
#include <QPixmapCache>
#include <QMetaObject>
#include <QTime>
#ifndef QT_NO_DRAGANDDROP
#include <QMimeData>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDropEvent>
#endif
#include <QHelpEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QStyle>
#include <QKeySequence>
#include <QClipboard>
#include <QToolTip>
#include <QDesktopWidget>

namespace {

    static const QRgb tileLight = qRgb( 0x9C, 0x95, 0x9C );
    static const QRgb tileDark  = qRgb( 0x62, 0x65, 0x63 );
    static const int tileSize = 5;

#ifndef QT_NO_CLIPBOARD
    static bool canPaste() {
        const QMimeData * const mime = QApplication::clipboard()->mimeData();
        return mime && mime->hasColor();
    }
#endif

#ifndef QT_NO_COLORDIALOG
    // work around QColorDialog's confusing API when dealing with alpha channel:
    static QColor getColorInternal( bool allowTransparentColors, const QColor & color, QWidget * parent ) {
        if ( allowTransparentColors ) {
            bool ok = false;
            const QRgb rgba = QColorDialog::getRgba( color.rgba(), &ok, parent );
            if ( !ok )
                return QColor(); // canceled->invalid
            QColor newColor; // QColor( QRgb ) ignores alpha channel!
            newColor.setRgba( rgba );
            return newColor;
        } else {
            return QColorDialog::getColor( color, parent );
        }
    }
#endif // QT_NO_COLORDIALOG

    static bool activateOnSingleClick( const QStyle * style ) {
        return style->styleHint( QStyle::SH_ItemView_ActivateItemOnSingleClick );
    }

    static bool activateOnDoubleClick( const QStyle * style ) {
        return !activateOnSingleClick( style );
    }

    static bool testAlwaysShowToolTipsAttribute( const QWidget & w ) {
#if QT_VERSION < 0x040300
        return kdCheckQtVersion( 4, 3 ) && w.testAttribute( static_cast<Qt::WidgetAttribute>( 84 ) /*Qt::WA_AlwaysShowToolTips*/ );
#else
        return w.testAttribute( Qt::WA_AlwaysShowToolTips );
#endif
    }

    static bool showComputedToolTip( const KDColorButton & cb ) {
        return cb.toolTip().isEmpty() && ( cb.isActiveWindow() || testAlwaysShowToolTipsAttribute( cb ) );
    }
}

/*!
  \class KDColorButton KDColorButton
  \ingroup gui
  \brief A color request/display widget

  A KDColorButton is a simple widget that allows the user to choose a
  color and, optionally, an alpha (transparency) value. Whenever the
  current color changes, the colorChanged() signal is emitted.

  You can allow or disallow transparency values using the
  #allowTransparentColors property. For convenience, if you pass a
  transparent color to the constructor, #allowTransparentColors is
  enabled automatically. If you enable transparent colors, consider
  setting defaultBackgroundPixmap() as the widgets background
  (otherwise, the parent widget shines through, as usual for
  QWidgets).

  The widget allows to drag and drop color values, as well as copy()
  and paste() to the system clipboard. It also has
  pick-color-from-screen functionality.

  The user can \link unsetColor() unset\endlink the color, in which
  case the contained #color will be \link QColor::isValid()
  invalid\endlink. This can be disabled using the #allowInvalidColors
  property.

  Note that #allowInvalidColors and #allowTransparentColors only
  restrict what colors the \em user can select. Values passed to
  setColor() are not checked. This way, it is possible to request a
  color without providing a default. However, all drops, pastes and
  selects are checked against these restrictions.

  \sa QColorDialog
*/

class KDColorButton::Private {
    friend class ::KDColorButton;
    KDColorButton * const q;
public:
    Private( const QColor & c, KDColorButton * qq );
    ~Private();

private:
    void retranslateUi();
#if !defined(QT_NO_DRAGANDDROP) || !defined(QT_NO_CLIPBOARD)
    QMimeData * makeMimeData() const  {
        QMimeData * const mime = new QMimeData;
        mime->setColorData( color );
        return mime;
    }
    QColor extractColor( const QMimeData * mime ) const {
        QColor c = qVariantValue<QColor>( mime->colorData() );
        if ( !allowInvalidColors && !c.isValid() )
            c = Qt::black;
        if ( !allowTransparentColors )
            c.setAlpha( 255 );
        return c;
    }
#endif
    QString makeToolTip() const;

    void startPicking();
    void stopPicking( bool accept );

private: // slots
    void _kd_enableDisableActions();

private:
    QAction requestColorAction;
    QAction unsetColorAction;
    QAction pickScreenColorAction;
#ifndef QT_NO_CLIPBOARD
    QAction separatorAction;
    QAction copyAction;
    QAction pasteAction;
#endif

    QColor color;
    bool   allowTransparentColors : 1;
    bool   allowInvalidColors     : 1;

    /* transient */ bool picking  : 1;
    /* transient */ bool sawKeyPressOpenRequest : 1;
    /* transient */ QPoint mousePressPosition;
    /* transient */ QTime  mousePressTime;
    /* transient */ QColor prePickingColor;
};

KDColorButton::Private::Private( const QColor & c, KDColorButton * qq )
    : q( qq ),
      requestColorAction( q ),
      unsetColorAction( q ),
      pickScreenColorAction( q ),
#ifndef QT_NO_CLIPBOARD
      separatorAction( q ),
      copyAction( q ),
      pasteAction( q ),
#endif
      color( c ),
      // allow transparent color by default if the color being passed
      // in has non-solid transparency:
      allowTransparentColors( c.isValid() && c.alpha() != 255 ),
      allowInvalidColors( true ),
      picking( false ),
      sawKeyPressOpenRequest( false ),
      mousePressPosition(),
      mousePressTime(),
      prePickingColor()
{
    KDAB_SET_OBJECT_NAME( requestColorAction );
    KDAB_SET_OBJECT_NAME( unsetColorAction );
    KDAB_SET_OBJECT_NAME( pickScreenColorAction );
#ifndef QT_NO_CLIPBOARD
    KDAB_SET_OBJECT_NAME( separatorAction );
    KDAB_SET_OBJECT_NAME( copyAction );
    KDAB_SET_OBJECT_NAME( pasteAction );
#endif

    unsetColorAction.setShortcut( Qt::Key_Escape );
    unsetColorAction.setShortcutContext( Qt::WidgetShortcut );
#ifndef QT_NO_CLIPBOARD
    separatorAction.setSeparator( true );
#if QT_VERSION < 0x040200
    copyAction.setShortcut( Qt::CTRL+Qt::Key_C );
    pasteAction.setShortcut( Qt::CTRL+Qt::Key_V );
#else
    copyAction.setShortcut( QKeySequence::Copy );
    pasteAction.setShortcut( QKeySequence::Paste );
#endif
    copyAction.setShortcutContext( Qt::WidgetShortcut );
    pasteAction.setShortcutContext( Qt::WidgetShortcut );
    pasteAction.setEnabled( canPaste() );
#endif

    connect( &requestColorAction, SIGNAL(triggered()),
             q, SLOT(requestColor()) );
    connect( &unsetColorAction, SIGNAL(triggered()),
             q, SLOT(unsetColor()) );
    connect( &pickScreenColorAction, SIGNAL(triggered()),
             q, SLOT(pickScreenColor()) );
#ifndef QT_NO_CLIPBOARD
    connect( &copyAction, SIGNAL(triggered()),
             q, SLOT(copy()) );
    connect( &pasteAction, SIGNAL(triggered()),
             q, SLOT(paste()) );
#endif

    q->addAction( &requestColorAction );
    q->addAction( &unsetColorAction );
    q->addAction( &pickScreenColorAction );
#ifndef QT_NO_CLIPBOARD
    q->addAction( &separatorAction );
    q->addAction( &copyAction );
    q->addAction( &pasteAction );
#endif
    q->setContextMenuPolicy( Qt::ActionsContextMenu );

    q->setAcceptDrops( true );
    q->setFocusPolicy( Qt::StrongFocus );

#ifndef QT_NO_CLIPBOARD
    connect( QApplication::clipboard(), SIGNAL(dataChanged()),
             q, SLOT(_kd_enableDisableActions()) );
#endif

    retranslateUi();
    _kd_enableDisableActions();
}

KDColorButton::Private::~Private() {}

void KDColorButton::Private::retranslateUi() {
    requestColorAction.setText( tr( "C&hange Color..." ) );
    unsetColorAction.setText( tr( "&Unset Color" ) );
    pickScreenColorAction.setText( tr( "Pick a Color from &Screen" ) );
#ifndef QT_NO_CLIPBOARD
    copyAction.setText( tr( "&Copy" ) ); // ### use std texts from Qt?
    pasteAction.setText( tr( "&Paste" ) );
#endif
}

QString KDColorButton::Private::makeToolTip() const {
    if ( !color.isValid() )
        return tr( "No color selected" );
    if ( color.alpha() != 255 )
        return tr( "Color: %1 (%2% opacity)" )
            .arg( color.name(), QString().sprintf( "%.1f", color.alphaF() * 100.0 )  );
    else
        return tr( "Color: %1" ).arg( color.name() );
}

void KDColorButton::Private::startPicking() {
    if ( picking )
        return;
    if ( !q->isVisible() )
        return; // Qt docs say that grabMouse() only works for visible widgets
    prePickingColor = color;
    picking = true;
#ifndef QT_NO_CURSOR
    q->grabMouse( Qt::CrossCursor );
#else
    q->grabMouse();
#endif // QT_NO_CURSOR
    q->grabKeyboard();
    q->setMouseTracking( true );
#if QT_VERSION < 0x040200
    q->setContextMenuPolicy( Qt::NoContextMenu );
#else
    q->setContextMenuPolicy( Qt::PreventContextMenu );
#endif
}

void KDColorButton::Private::stopPicking( bool accept ) {
    if ( !picking )
        return;

    q->releaseMouse();
    q->releaseKeyboard();
    q->setMouseTracking( false );
    q->setContextMenuPolicy( Qt::ActionsContextMenu );
    picking = false;

    if ( accept ) {
        const QColor tmp = color;
        color = QColor();
        q->setColor( tmp );
    } else {
        color = prePickingColor;
        q->update();
    }
}

// private slot
void KDColorButton::Private::_kd_enableDisableActions() {
    unsetColorAction.setEnabled( allowInvalidColors );
#ifndef QT_NO_CLIPBOARD
    pasteAction.setEnabled( canPaste() );
#endif
}

/*!
  Constructor. Constructs a KDColorButton containing an invalid color.
  The \a p and \a f arguments are passed down to the QWidget
  constructor.
*/
KDColorButton::KDColorButton( QWidget * p, Qt::WFlags f )
    : QWidget( p, f ), d( new Private( QColor(), this ) )
{

}

/*!
  Constructor. Constructs a KDColorButton containing color \a c.  The
  \a p and \a f arguments are passed down to the QWidget constructor.
*/
KDColorButton::KDColorButton( const QColor & c, QWidget * p, Qt::WFlags f )
    : QWidget( p, f ), d( new Private( c, this ) )
{

}

/*!
  Destructor.
*/
KDColorButton::~KDColorButton() {
    d->stopPicking( false );
}


/*!
  Returns a background pixmap that is suitable for KDColorButton's
  with allwed transparent colors. It is also used to indicate an
  invalid color, should one be set.

  To set this pixmap as the background for a KDColorButton, do this:
  \code
  QPalette pal;
  pal.setBrush( colorButton->backgroundRole(), KDColorButton::defaultBackgroundPixmap() );
  colorButton->setPalette( pal );
  colorButton->setAutoFillBackground( true ); // background is otherwise ignored
  \endcode

  \todo decide whether this can be in any way switched on and off
  automatically w/o constricting the class user too much.
*/
// static
QPixmap KDColorButton::defaultBackgroundPixmap() {
    static const QString key = QLatin1String( "KDColorButton::defaultBackgroundPixmap()" );
    {
        QPixmap cached;
        if ( QPixmapCache::find( key, cached ) )
            return cached;
    }
    QPixmap pm( tileSize * 2, tileSize * 2 );
    pm.fill( tileDark );
    QPainter p( &pm );
    p.setBrush( QColor( tileLight ) );
    p.setPen( Qt::NoPen );
    p.drawRect( 0, 0, tileSize, tileSize );
    p.drawRect( tileSize, tileSize, tileSize, tileSize );
    p.end();
    QPixmapCache::insert( key, pm );
    return pm;
}

/*!
  Returns the pixel color at position \a p on the screen.

  Note that the returned color will never have transparency.
*/
// static
QColor KDColorButton::screenPixel( const QPoint & p ) {
    return QPixmap::grabWindow( QApplication::desktop()->winId(), p.x(), p.y(), 1, 1 ).toImage().pixel( 0, 0 );
}

/*!
  \property KDColorButton::allowTransparentColors

  Specifies whether the user should be allowed to choose transparent
  colors (QColor::alpha() != 255), or not. The default is \c false,
  unless a transparent color is passed to the \link
  KDColorButton(const QColor&,QWidget*,Qt::WFlags) KDColorButton
  constructor\endlink, in which case it defaults to \c true.

  If transparent colors are disallowed, and a transparent color is
  received in a drop or paste operation, the alpha channel will be
  adjusted to full opacity before accepting the value. However, all
  alpha values are accepted as they are in setColor() calls.

  Get this property's value using %allowTransparentColors(), and set
  it using %setAllowTransparentColors().

  \todo decide whether to switch this on when programmatically setting
  a transparent color with %setColor().
*/

bool KDColorButton::allowTransparentColors() const {
    return d->allowTransparentColors;
}

/*!
  See #allowTransparentColors.
*/
void KDColorButton::setAllowTransparentColors( bool on ) {
    if ( on == d->allowTransparentColors )
        return;
    d->allowTransparentColors = on;
    update(); // ### not needed, or is it?
}

/*!
  \property KDColorButton::allowInvalidColors

  Specifies whether the user should be allowed to reset the widget to
  an invalid color (cf. QColor::isValid(), or not. The default is \c
  true.

  If invalid colors are disallowed, and an invalid color is received
  from a drop or paste operation, it is replaced by Qt::black
  instead. However, invalid colors are accepted as they are in
  setColor() calls.

  Get this property's value using %allowInvalidColors(), and set it
  using %setAllowInvalidColors().
*/

bool KDColorButton::allowInvalidColors() const {
    return d->allowInvalidColors;
}

/*!
  See #allowInvalidColors.
*/
// slot
void KDColorButton::setAllowInvalidColors( bool on ) {
    if ( on == d->allowInvalidColors )
        return;
    d->allowInvalidColors = on;
    d->_kd_enableDisableActions();
    update(); // ### not needed, or is it?
}

/*!
  \property KDColorButton::color

  Contains the currently selected color. The default is an invalid
  color, or whatever color has been passed to the \link
  KDColorButton(const QColor&,QWidget*,Qt::WFlags) KDColorButton
  constructor\endlink.

  Get this property's value using %color(), monitor it using the
  colorChanged() signal, and set it using %setColor().
*/

QColor KDColorButton::color() const {
    return d->color;
}

/*!
  See #color.
*/
void KDColorButton::setColor( const QColor & c ) {
    if ( c == d->color )
        return;
    d->color = c;
    update();
    emit colorChanged( c );
}

/*!
  Programmatically initiates a color request by opening the color
  selection dialog. The new color (if any) has been set when this
  function returns.

  You probably never need to call this yourself.
*/
// slot
void KDColorButton::requestColor() {
    const QColor newColor = getColor();
    if ( !newColor.isValid() )
        return; // canceled -> do nothing
    setColor( newColor );
}

/*!
  Invalidates (unsets) the current color. Equivalent to
  \code
  setColor( QColor() )
  \endcode
  however, this functions checks #allowInvalidColors before proceeding.
*/
// slot
void KDColorButton::unsetColor() {
    if ( d->allowInvalidColors )
        setColor( QColor() );
}

/*!
  Lets the user pick a pixel color from the screen. This simply starts
  the operation, the new pixel value will not have been set yet when
  this function returns.

  During color picking, the mouse and keyboard are grabbed. The left
  mouse button selects a new color, the right mouse button, as well as
  the Escape key, cancel the operation.
*/
void KDColorButton::pickScreenColor() {
    d->startPicking();
}

#ifndef QT_NO_CLIPBOARD
/*!
  Copies the current color to the system clipboard.
*/
// slot
void KDColorButton::copy() {
    QApplication::clipboard()->setMimeData( d->makeMimeData() );
}

/*!
  If possible, pastes the current color from the system
  clipboard. This function does nothing if the clipboard \link
  QMimeData::hasColor() doesn't contain a color\endlink.
*/
// slot
void KDColorButton::paste() {
    if ( canPaste() )
        setColor( d->extractColor( QApplication::clipboard()->mimeData() ) );
}
#endif // QT_NO_CLIPBOARD

/*!
  \fn void KDColorButton::colorChanged( const QColor & color )

  Emitted whenever the \link #color current color\endlink changes, both
  programmatically, as well as through user interaction.
*/

/*!
  \fn QColor KDColorButton::getColor()

  Reimplement this virtual function to control the way colors are
  requested from the user. The default implementation pops up a
  QColorDialog. On user cancel, return an invalid QColor.

  If QColorDialog isn't available, this function is pure virtual, and
  you \em must implement it in a subclass of KDColorButton.
*/
#ifndef QT_NO_CLIPBOARD
QColor KDColorButton::getColor() {
    return getColorInternal( d->allowTransparentColors, d->color, this );
}
#endif

QSize KDColorButton::minimumSizeHint() const {
    return QSize( 10, 10 ); // FIXME
}

QSize KDColorButton::sizeHint() const {
    return QSize( 20, 20 ); // FIXME
}

bool KDColorButton::event( QEvent * e ) {
    switch ( e->type() ) {
    case QEvent::ToolTip:
        if ( showComputedToolTip( *this ) ) {
            // user didn't set a tooltip, so we provide one:
            QToolTip::showText( static_cast<QHelpEvent*>( e )->globalPos(), d->makeToolTip(), this );
            return true;
        }
        // fall through:
    default:
        return QWidget::event( e );
    }
}

void KDColorButton::paintEvent( QPaintEvent * ) {
    // FIXME: how should this look like?
    QPainter p( this );
    if ( d->color.isValid() )
        p.setBrush( d->color );
    else
        p.setBrush( defaultBackgroundPixmap() );
    if ( hasFocus() )
        p.setPen( Qt::black );
    else
        p.setPen( d->color );
    p.drawRect( rect().adjusted( 0, 0, -1, -1 ) );
}

void KDColorButton::keyPressEvent( QKeyEvent * e ) {
    switch ( e->key() ) {
    case Qt::Key_Return:
    case Qt::Key_Enter:
    case Qt::Key_Select:
    case Qt::Key_Space:
	d->sawKeyPressOpenRequest = true;
        break;
    case Qt::Key_Escape:
        if ( d->picking ) {
            d->stopPicking( false );
            e->accept();
            break;
        }
    default:
	d->sawKeyPressOpenRequest = false;
        break;
    }
}

void KDColorButton::keyReleaseEvent( QKeyEvent * e ) {
    switch ( e->key() ) {
    case Qt::Key_Return:
    case Qt::Key_Enter:
    case Qt::Key_Select:
    case Qt::Key_Space:
	if ( d->sawKeyPressOpenRequest ) {
	    QMetaObject::invokeMethod( this, "requestColor", Qt::QueuedConnection );
	    e->accept();
	    break;
	}
    default:
        e->ignore();
        break;
    }
    d->sawKeyPressOpenRequest = false;
}

void KDColorButton::mousePressEvent( QMouseEvent * e ) {
    if ( d->picking ) {
        e->accept();
        return;
    }
    if ( e->button() != Qt::LeftButton )
        return;
    d->mousePressPosition = e->pos();
    d->mousePressTime.start();
}

void KDColorButton::mouseMoveEvent( QMouseEvent * e ) {

    if ( d->picking ) {
        d->color = screenPixel( e->globalPos() );
        e->accept();
        update();
        return;
    }

#ifndef QT_NO_DRAGANDDROP
    if ( !( e->buttons() & Qt::LeftButton) )
        return;
    if ( ( e->pos() - d->mousePressPosition ).manhattanLength() < QApplication::startDragDistance() &&
         d->mousePressTime.elapsed() < QApplication::startDragTime() )
        return;

    QDrag * const drag = new QDrag( this );

    drag->setMimeData( d->makeMimeData() );

    (void)drag->start( Qt::CopyAction );
#endif // QT_NO_DRAGANDDROP
}

void KDColorButton::mouseReleaseEvent( QMouseEvent * e ) {

    if ( d->picking ) {
        d->color = screenPixel( e->globalPos() );
        if ( e->button() == Qt::LeftButton )
            d->stopPicking( true );
        else
            d->stopPicking( false );
        e->accept();
        return;
    }

    if ( e->button() != Qt::LeftButton || !activateOnSingleClick( style() ) )
        return;
    QMetaObject::invokeMethod( this, "requestColor", Qt::QueuedConnection );
    e->accept();
}

void KDColorButton::mouseDoubleClickEvent( QMouseEvent * e ) {
    if ( d->picking ) {
        e->accept();
        return;
    }
    if ( e->button() != Qt::LeftButton || !activateOnDoubleClick( style() ) )
        return;
    QMetaObject::invokeMethod( this, "requestColor", Qt::QueuedConnection );
    e->accept();
}

#ifndef QT_NO_DRAGANDDROP
void KDColorButton::dragEnterEvent( QDragEnterEvent * e ) {
    if ( e->mimeData()->hasColor() )
        e->acceptProposedAction();
}

void KDColorButton::dropEvent( QDropEvent * e ) {
    const QMimeData * const mime = e->mimeData();
    if ( !mime || !mime->hasColor() )
        return;
    if ( !( e->possibleActions() & Qt::CopyAction ) )
        return;
    e->setDropAction( Qt::CopyAction );
    e->accept();
    setColor( d->extractColor( mime ) );
}
#endif // QT_NO_DRAGANDDROP

void KDColorButton::changeEvent( QEvent * e ) {  // FIXME: palette change?
    switch ( e->type() ) {
    case QEvent::LanguageChange:
        d->retranslateUi();
        break;
    default:
        break;
    }
    QWidget::changeEvent( e );
}

#include "moc_kdcolorbutton.cpp"
