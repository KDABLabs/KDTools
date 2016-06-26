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

#include "kdsearchlineedit.h"

#include "macwidget_p.h"

#include <QFile>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOptionFrame>

#ifdef Q_OS_MAC
#include <QMacCocoaViewContainer>
QMacCocoaViewContainer* createNSSearchField( QWidget* parent );
#endif

using namespace _macwidget;

/*!
 \internal
 */
class KDSearchLineEdit::Private
{
public:
    explicit Private( KDSearchLineEdit* q )
        : q( q ),
#ifdef Q_OS_MAC
          searchField( createNSSearchField( q ) ),
#endif
          menu( 0 ),
          internalLineEdit( q ),
          defaultText( tr( "Search..." ) ),
          clearButtonPressed( false ),
          clearButtonHovered( false )
    {
        internalLineEdit.installEventFilter( q );
        q->setFocusProxy( &internalLineEdit );
        _kdt_maybeShowDefaultText();

        connect( &internalLineEdit, SIGNAL(textEdited(QString)),
                 q, SLOT(_kdt_userEditedText(QString)) );
        connect( &internalLineEdit, SIGNAL(textEdited(QString)),
                 q, SIGNAL(textEdited(QString)) );
        connect( q, SIGNAL(textChanged(QString)),
                 &internalLineEdit, SLOT(setText(QString)) );
        connect( q, SIGNAL(textChanged(QString)),
                 q, SLOT(_kdt_maybeShowDefaultText()) );
        connect( &internalLineEdit, SIGNAL(editingFinished()),
                 q, SIGNAL(editingFinished()) );
        connect( &internalLineEdit, SIGNAL(returnPressed()),
                 q, SIGNAL(returnPressed()) );
    }

    ~Private()
    {
        internalLineEdit.removeEventFilter( q );
    }

private:
    KDSearchLineEdit* const q;

public:
    void moveInternalLineEdit();
    void _kdt_userEditedText( const QString& text );    
    void _kdt_maybeShowDefaultText();

    /*!
     Returns the rectangle containing the magnifier.
     \internal
     */
    QRect magnifierRect() const
    {
        const QPixmap pmMagnifier = isMacStyle( q->style() ) ? QPixmap( RESOURCE( "searchlineedit_magnifier" ) )
                                                             : QPixmap( RESOURCE( "searchlineedit_magnifier_win" ) );
        switch( q->layoutDirection() )
        {
        case Qt::RightToLeft:
            return isMacStyle( q->style() ) ? QRect( q->rect().topRight() - QPoint( pmMagnifier.width(), 0 ), pmMagnifier.size() )
#ifdef Q_WS_WIN
                                            : QRect( q->rect().topRight() - QPoint( pmMagnifier.width() + 2, -1 ), pmMagnifier.size() );
#else
                                            : QRect( q->rect().topRight() - QPoint( pmMagnifier.width() + 2, -2 ), pmMagnifier.size() );
#endif
        case Qt::LeftToRight:
        default:
            return isMacStyle( q->style() ) ? QRect( q->rect().topLeft(), pmMagnifier.size() ) 
#ifdef Q_WS_WIN
                                            : QRect( q->rect().topLeft() + QPoint( 2, 1 ), pmMagnifier.size() );
#else
                                            : QRect( q->rect().topLeft() + QPoint( 2, 2 ), pmMagnifier.size() );
#endif
        }
    }
   
    /*!
     Returns the rectangle containing the clear button.
     \internal
     */
    QRect clearButtonRect() const
    {
        const QPixmap pmClear = QPixmap( RESOURCE( "searchlineedit_clear.png" ) );
        switch( q->layoutDirection() )
        {
        case Qt::RightToLeft:
            return QRect( q->rect().topLeft(), pmClear.size() );
        case Qt::LeftToRight:
        default:
            return QRect( q->rect().topRight() - QPoint( pmClear.width(), 0 ), pmClear.size() );
        }
    }

#ifdef Q_OS_MAC
    QMacCocoaViewContainer* const searchField;
#endif

    QMenu* menu;
    QLineEdit internalLineEdit;
    QString defaultText;
    bool clearButtonPressed;
    bool clearButtonHovered;
};

/*!
 \class KDSearchLineEdit KDSearchLineEdit
 \ingroup gui macwidgets
 \brief A line edit usually used to type in text to search.

 A KDSearchLineEdit is a QLineEdit contaning a magnifier icon. Additionally, it can
 contain a default text being shown when the line edit has neither focus nor text.
*/

/*!
 Creates a new KDSearchLineEdit with parent \a parent.
 */
KDSearchLineEdit::KDSearchLineEdit( QWidget* parent )
    : QLineEdit( parent ), d( new Private( this ) )
{
}

/*!
  Creates a new KDSearchLineEdit with parent \a parent and text \a contents.
*/
KDSearchLineEdit::KDSearchLineEdit( const QString & contents, QWidget * parent )
    : QLineEdit( parent ), d( new Private( this ) )
{
    setText( contents );
}

/*!
 Destroys the KDSearchLineEdit.
 */
KDSearchLineEdit::~KDSearchLineEdit()
{
}

/*!
 \property KDSearchLineEdit::menu

 Contains the menu shown when the user clicks on the magnifier
 icon. If there's a menu defined, the magnifier icon shows a little
 arrow. The KDSearchLineEdit does not take ownership of the menu.

 Get this property's value using %menu(), and set it using %setMenu().
*/

void KDSearchLineEdit::setMenu( QMenu* menu )
{
    d->menu = menu;
#ifdef Q_OS_MAC
    d->searchField->setProperty( "menu", qVariantFromValue< QObject* >( menu ) );
#endif
    update();
}

QMenu* KDSearchLineEdit::menu() const
{
    return d->menu;
}

/*!
 \property KDSearchLineEdit::defaultText

 Contains the default text shown by KDSearchLineEdit. The default text
 is shown if the widget has no focus and does not contain any
 text. The default text is shown with the disabled-styled pen. The
 default value is "Search..."

 Get this property's value using %defaultText(), and set it using
 %setDefaultText().
*/

QString KDSearchLineEdit::defaultText() const
{
    return d->defaultText;
}

/*!
  See #defaultText.
*/
void KDSearchLineEdit::setDefaultText( const QString& defaultText )
{
    if( d->defaultText == defaultText )
        return;

    d->defaultText = defaultText;
#ifdef Q_OS_MAC
    d->searchField->setProperty( "defaultText", defaultText );
#endif
    d->_kdt_maybeShowDefaultText();
}

QSize KDSearchLineEdit::sizeHint() const
{
#ifdef Q_OS_MAC
    if( isMacStyle( style() ) )
    {
        const QPixmap pmLeft = QPixmap( RESOURCE( "searchlineedit_focus_left.png" ) );
        return QSize( QLineEdit::sizeHint().width(), pmLeft.height() );
    }
    else
#endif
    {
        const QPixmap pmLeft = QPixmap( RESOURCE( "searchlineedit_magnifier_win.png" ) );
        return QLineEdit::sizeHint() + QSize( pmLeft.width(), 0 );
    }
}

QSize KDSearchLineEdit::minimumSizeHint() const
{
#ifdef Q_OS_MAC
    if( isMacStyle( style() ) )
    {
        const QPixmap pmLeft = QPixmap( RESOURCE( "searchlineedit_focus_left.png" ) );
        const QPixmap pmRight = QPixmap( RESOURCE( "searchlineedit_focus_right.png" ) );
        return QSize( QLineEdit::minimumSizeHint().width() + pmLeft.width() + pmRight.width(), pmLeft.height() );
    }
    else
#endif
    {
        const QPixmap pmLeft = QPixmap( RESOURCE( "searchlineedit_magnifier_win.png" ) );
        return QLineEdit::minimumSizeHint() + QSize( pmLeft.width(), 0 );
    }
}

/*!
 Shows the default text if the line edit has no focus and does not contain any user text.
 \internal
 */
void KDSearchLineEdit::Private::_kdt_maybeShowDefaultText()
{
    QPalette p = internalLineEdit.palette();
    if( q->text().isEmpty() && !q->hasFocus() && !internalLineEdit.hasFocus() )
    {
        p.setColor( QPalette::Text, q->palette().color( QPalette::Disabled, QPalette::Text ) );
        internalLineEdit.setText( defaultText );
        internalLineEdit.setCursorPosition( 0 );
    }
    else
    {
        if( q->text().isEmpty() )
            internalLineEdit.clear();
        p.setColor( QPalette::Text, q->palette().color( QPalette::Active, QPalette::Text ) );
    }
    internalLineEdit.setPalette( p );
}

/*!
 Syncs the user edited text in the internal line edit with the "real" visible text.
 \internal
 */
void KDSearchLineEdit::Private::_kdt_userEditedText( const QString& text )
{
    const int cursor = internalLineEdit.cursorPosition();
    q->setText( text );
    internalLineEdit.setCursorPosition( cursor );
}

void KDSearchLineEdit::setText( const QString& text )
{
#ifdef Q_OS_MAC
    d->searchField->setProperty( "text", text );
#endif
    QLineEdit::setText( text );
}

bool KDSearchLineEdit::eventFilter( QObject* watched, QEvent* event )
{
    if( event->type() == QEvent::FocusIn || event->type() == QEvent::FocusOut )
    {
        d->_kdt_maybeShowDefaultText();
        update();
    }

    return QLineEdit::eventFilter( watched, event );
}

void KDSearchLineEdit::resizeEvent( QResizeEvent* event )
{
#ifdef Q_OS_MAC
    d->searchField->resize( size() );
#endif
    d->moveInternalLineEdit();
    QLineEdit::resizeEvent( event );
}

/*!
 Moves the internal line edit so that it fits between the icon and the clear button.
 \internal
 */
void KDSearchLineEdit::Private::moveInternalLineEdit()
{
#ifdef Q_OS_MAC
    if( isMacStyle( q->style() ) )
    {
        const QPixmap pmLeft  = QPixmap( RESOURCE( "searchlineedit_left.png" ) );
        const QPixmap pmRight = QPixmap( RESOURCE( "searchlineedit_right.png" ) );
    
        internalLineEdit.setGeometry( pmLeft.width(), 4,
                                      q->width() - pmLeft.width() - pmRight.width(), pmRight.height() - 2 * 4 );
    }
    else
#endif
    {
        QStyleOptionFrameV2 option;
        option.initFrom( q );
        option.rect = q->contentsRect();
        option.lineWidth = q->hasFrame() ? q->style()->pixelMetric( QStyle::PM_DefaultFrameWidth, &option, q ) : 0;
        QRect geo = q->style()->subElementRect( QStyle::SE_LineEditContents, &option, q );
        const QPixmap pmLeft = QPixmap( RESOURCE( "searchlineedit_magnifier_win.png" ) );
        switch( q->layoutDirection() )
        {
        case Qt::RightToLeft:
            geo.setRight( geo.right() - pmLeft.width() - 2 );
            break;
        case Qt::LeftToRight:
        default:
            geo.setLeft( geo.left() + pmLeft.width() + 2 );
            break;
        }
        internalLineEdit.setGeometry( geo );
    }
}

void KDSearchLineEdit::mousePressEvent( QMouseEvent* event )
{
#ifdef Q_OS_MAC
    if( isMacStyle( style() ) )
    {
        d->clearButtonPressed = d->clearButtonRect().contains( event->pos() );
        d->clearButtonHovered = d->clearButtonRect().contains( event->pos() );
        update();
    }
#endif

    if( d->magnifierRect().contains( event->pos() ) && d->menu != 0 )
    {
        switch( layoutDirection() )
        {
        case Qt::RightToLeft: 
            d->menu->exec( mapToGlobal( d->magnifierRect().bottomRight() + QPoint( -d->menu->sizeHint().width() - 4, -2 ) ) );
            break;
        case Qt::LeftToRight: 
        default:
            d->menu->exec( mapToGlobal( d->magnifierRect().bottomLeft() + QPoint( 4, -2 ) ) );
            break;
        }
    }
    QLineEdit::mousePressEvent( event );
}

void KDSearchLineEdit::mouseMoveEvent( QMouseEvent* event )
{
#ifdef Q_OS_MAC
    if( isMacStyle( style() ) )
        d->clearButtonHovered = d->clearButtonRect().contains( event->pos() );
#endif
    QLineEdit::mouseMoveEvent( event );
}

void KDSearchLineEdit::mouseReleaseEvent( QMouseEvent* event )
{
#ifdef Q_OS_MAC
    if( isMacStyle( style() ) && d->clearButtonPressed && d->clearButtonRect().contains( event->pos() ) )
    {
        clear();
        emit textEdited( QString() );
    }
#endif
    d->clearButtonHovered = false;
    d->clearButtonPressed = false;
    update();
    QLineEdit::mouseReleaseEvent( event );
}

void KDSearchLineEdit::paintEvent( QPaintEvent* event )
{
    Q_UNUSED( event );

    d->internalLineEdit.setVisible( true );
    if (d->internalLineEdit.hasFrame())
        d->internalLineEdit.setFrame( false );
    d->internalLineEdit.setAttribute( Qt::WA_MacShowFocusRect, false );
    setAttribute( Qt::WA_MacShowFocusRect, false );
        
    QPainter p( this );

#ifdef Q_OS_MAC
    const bool mac = isMacStyle( style() );
    d->searchField->setVisible( mac );
    d->internalLineEdit.setVisible( !mac );
    
    if( !mac )
#endif // Q_OS_MAC
    {
        QStyleOptionFrameV2 panel;
        initStyleOption( &panel );
        if( d->internalLineEdit.hasFocus() )
            panel.state |= QStyle::State_HasFocus;
        style()->drawPrimitive( QStyle::PE_PanelLineEdit, &panel, &p, this );
        const QPixmap pmMagnifier = d->menu == 0 ? QPixmap( RESOURCE( "searchlineedit_magnifier_win.png" ) )
                                                 : QPixmap( RESOURCE( "searchlineedit_magnifier_win_arrow.png" ) );

        p.drawPixmap( d->magnifierRect(), pmMagnifier );
    }
}

#include "moc_kdsearchlineedit.cpp"
