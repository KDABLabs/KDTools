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

#include "kdlistviewtoolbar.h"

#include "macwidget_p.h"

#include <QAbstractButton>
#include <QAction>
#include <QActionEvent>
#include <QApplication>
#include <QColor>
#include <QFile>
#include <QGradient>
#include <QMenu>
#include <QPainter>
#include <QPen>
#include <QStyleOptionToolButton>
#include <QStylePainter>

using namespace _macwidget;

// the default arrow width
static const int ArrowWidth = 6;

class KDListViewToolButton : public QAbstractButton
{
    Q_OBJECT
public:
    KDListViewToolButton( KDListViewToolBar * parent, QAction * action )
        : QAbstractButton( parent ),
          m_showsMenu( false ),
          m_action( action )
    {
        connect( m_action, SIGNAL(destroyed()), this, SLOT(deleteLater()) );
        connect( m_action, SIGNAL(changed()), this, SLOT(_kdt_actionChanged()) );
        connect( this, SIGNAL(clicked()), m_action, SLOT(trigger()) );
        setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
        _kdt_actionChanged();
    }

    QSize sizeHint() const KDAB_OVERRIDE;

#ifndef QT_NO_MENU
    bool hasMenu() const
    {
        return m_action->menu() != 0;
    }
#endif

protected:
    void paintEvent( QPaintEvent* event ) KDAB_OVERRIDE;
    void mousePressEvent( QMouseEvent* event ) KDAB_OVERRIDE;

    void initStyleOption( QStyleOptionToolButton* option ) const;

private Q_SLOTS:
    /*!
     Redraws the button and tells the toolbar to relayout when the QAction changes.
    */
    void _kdt_actionChanged()
    {
        setVisible( m_action->isVisible() );
        setEnabled( m_action->isEnabled() );
        setToolTip( m_action->toolTip() );
        setText( m_action->text() );
        if( text() == QLatin1String( "*" ) )
        {
            setText( QString() );
            setIcon( QIcon( RESOURCE( "toolbutton_options.png" ) ) );
        }
        updateGeometry();
        QMetaObject::invokeMethod( parent(), "_kdt_layoutButtons" );
        update();
    }

    /*!
     Makes sure the button is shown as not pressed when the menu
     dissapeares.
     \internal
    */
    void _kdt_menuAboutToHide()
    {
        m_showsMenu = false;
        update();
    }

public:
    bool m_showsMenu;
    QAction * const m_action;
};

void KDListViewToolButton::initStyleOption(QStyleOptionToolButton *option) const
{
    if (!option)
        return;

    option->initFrom(this);
    bool forceNoText = false;
    option->iconSize = iconSize(); //default value

    if (!forceNoText)
        option->text = text();
    option->icon = icon();
    if (isDown())
        option->state |= QStyle::State_Sunken;
    if (isChecked())
        option->state |= QStyle::State_On;
    if (!isChecked() && !isDown() )
        option->state |= QStyle::State_Raised;

    option->subControls = QStyle::SC_ToolButton;
    option->activeSubControls = QStyle::SC_None;

    option->features = QStyleOptionToolButton::None;
    /*if (d->popupMode == QToolButton::MenuButtonPopup) {
        option->subControls |= QStyle::SC_ToolButtonMenu;
        option->features |= QStyleOptionToolButton::MenuButtonPopup;
    }
    if (option->state & QStyle::State_MouseOver) {
        option->activeSubControls = d->hoverControl;
    }*/
    if (isDown()) {
        option->state |= QStyle::State_Sunken;
        option->activeSubControls |= QStyle::SC_ToolButton;
    }


    //if (arrowType() != Qt::NoArrow)
    //    option->features |= QStyleOptionToolButton::Arrow;
    //if (popupMode() == QToolButton::DelayedPopup)
    //    option->features |= QStyleOptionToolButton::PopupDelay;
    option->arrowType = Qt::NoArrow;
#ifndef QT_NO_MENU
    if( hasMenu() )
    {
        option->features |= QStyleOptionToolButton::HasMenu;
        option->arrowType = Qt::DownArrow;
    }
#endif
    //option->toolButtonStyle = d->toolButtonStyle;
    if (icon().isNull() /*&& d->arrowType == Qt::NoArrow*/ && !forceNoText) {
        if (!text().isEmpty())
            option->toolButtonStyle = Qt::ToolButtonTextOnly;
        else if (option->toolButtonStyle != Qt::ToolButtonTextOnly)
            option->toolButtonStyle = Qt::ToolButtonIconOnly;
    } else {
        if (text().isEmpty() && option->toolButtonStyle != Qt::ToolButtonIconOnly)
            option->toolButtonStyle = Qt::ToolButtonIconOnly;
    }

    option->pos = pos();
    option->font = font();
}

QSize KDListViewToolButton::sizeHint() const
{
#ifdef Q_OS_MAC
    if ( isMacStyle( style() ) )
    {

        const int height = 22;

        QFont f = m_action->font();
        f.setWeight( text().size() <= 1 ? QFont::Black : QFont::Normal );

        const QFontMetrics fm( f );
        int width = fm.width( text() ) + 8;

        if ( m_action->isSeparator() )
            width = 12;

        if( !icon().isNull() && !text().isEmpty() )
            width += 16;

        if ( !m_action->isSeparator() )
            width = qMax( 22, width );

#ifndef QT_NO_MENU
        if( hasMenu() )
            width += ArrowWidth;
#endif

        const QList< QAction* > actions = parentWidget()->actions();
        const bool isLastButton = actions.last() == m_action || actions[ actions.indexOf( m_action ) + 1 ]->isSeparator();
        // if this is the last button in a group (in the toolbar or followed by a separator)
        // we need one more pixel space for the right border
        if( isLastButton )
            width += 1;

        return QSize( width, height );

    }
    else
#endif // Q_OS_MAC
    {
        ensurePolished();

        int w = 0, h = 0;
        QStyleOptionToolButton opt;
        initStyleOption(&opt);

        QFontMetrics fm = fontMetrics();
        if (opt.toolButtonStyle != Qt::ToolButtonTextOnly) {
            QSize icon = opt.iconSize;
            w = icon.width();
            h = icon.height();
        }

        if (opt.toolButtonStyle != Qt::ToolButtonIconOnly) 
        {
            QSize textSize = fm.size(Qt::TextShowMnemonic, text());
            textSize.setWidth(textSize.width() + fm.width(QLatin1Char(' '))*0);
            if (opt.toolButtonStyle == Qt::ToolButtonTextUnderIcon) {
                h += 4 + textSize.height();
                if (textSize.width() > w)
                    w = textSize.width();
            } else if (opt.toolButtonStyle == Qt::ToolButtonTextBesideIcon) {
                w += 4 + textSize.width();
                if (textSize.height() > h)
                    h = textSize.height();
            } else { // TextOnly
                w = textSize.width();
                h = textSize.height();
            }
        
        }

        w = qMax( w, h );
        if( m_action->isSeparator() )
            return QSize( 12, h );

        opt.rect.setSize(QSize(w, h)); // PM_MenuButtonIndicator depends on the height
#ifndef QT_NO_MENU
        if( hasMenu() )
            w += style()->pixelMetric(QStyle::PM_MenuButtonIndicator, &opt, this);
#endif

        return style()->sizeFromContents(QStyle::CT_ToolButton, &opt, QSize(w, h), this).
                      expandedTo(QApplication::globalStrut());
    }
}

void KDListViewToolButton::mousePressEvent( QMouseEvent* event )
{
    if( event->button() == Qt::LeftButton && hasMenu() && !m_action->menu()->isVisible() )
    {
        connect( m_action->menu(), SIGNAL(aboutToHide()), this, SLOT(_kdt_menuAboutToHide()) );
        m_showsMenu = true;
        update();
        m_action->menu()->exec( mapToGlobal( rect().bottomLeft() ) );
        disconnect( m_action->menu(), SIGNAL(aboutToHide()), this, SLOT(_kdt_menuAboutToHide()) );
        return;
    }
    QAbstractButton::mousePressEvent( event );
}

void KDListViewToolButton::paintEvent( QPaintEvent* event )
{
    Q_UNUSED( event )

    if( m_action->isSeparator() )
        return;

#ifdef Q_OS_MAC
    if ( isMacStyle( style() ) )
    {
        QPainter p( this );

        const bool down = isDown() || m_showsMenu;

        QRect r = rect();

        const QList< QAction* > actions = parentWidget()->actions();
        const bool isLastButton = actions.last() == m_action || actions[ actions.indexOf( m_action ) + 1 ]->isSeparator();

        // top border
        p.setPen( QPen( QColor( 213, 213, 213 ) ) );
        p.drawLine( r.topLeft(), r.topRight() );

        // bottom border
        p.setPen( QPen( QColor( 243, 243, 243 ) ) );
        p.drawLine( r.bottomLeft(), r.bottomRight() );

        // left border
        p.setPen( QPen( QColor( 131, 131, 131 ) ) );
        p.drawLine( r.left(), r.top() + 1, r.left(), r.bottom() - 1 );

        // bottom border and top shadow border
        p.setPen( QPen( QColor( 151, 151, 151 ) ) );
        p.drawLine( r.left(), r.bottom() - 1, r.right(), r.bottom() - 1 );
        p.drawLine( r.left(), r.top() + 2, r.left(), r.bottom() - 1 );

        // gradient for the content, button down or not down
        QLinearGradient gradient( r.left() + 1, r.center().y() + 1, r.left() + 1, r.bottom() - 2 );
        gradient.setColorAt( 0.0, down ? QColor( 165, 165, 165 ) : QColor( 237, 237, 237 ) );
        gradient.setColorAt( 1.0, down ? QColor( 170, 170, 170 ) : QColor( 243, 243, 243 ) );
        p.fillRect( r.left() + 1, r.top() + 2, r.width(), r.height() / 2, QBrush( down ? QColor( 175, 175, 175 ) : QColor( 251, 251, 251 ) ) );
        p.fillRect( r.left() + 1, r.center().y() + 1, r.width(), r.height() / 2 - 2, gradient );

        gradient.setStart( r.left() + 1, r.top() + 1 );
        gradient.setFinalStop( r.right(), r.top() + 1 );
        gradient.setColorAt( 0.0, QColor( 124, 124, 124 ) );
        gradient.setColorAt( 1.0, QColor( 127, 127, 127 ) );
        p.fillRect( r.left() + 1, r.top() + 1, width(), 1, gradient );

        if ( isLastButton )
        {
            // if this is the last button in a group (in the toolbar or followed by a separator)
            // we draw the right border
            p.setPen( QPen( QColor( 131, 131, 131 ) ) );
            p.drawLine( r.right(), r.top() + 1, r.right(), r.bottom() - 1 );
            p.setPen( QPen( QColor( 151, 151, 151 ) ) );
            p.drawLine( r.right(), r.top() + 2, r.right(), r.bottom() - 1 );
        }

#ifndef QT_NO_MENU
        if ( hasMenu() )
        {
            // draws the little arrow signaling the menu
            QFont f;
            f.setPointSizeF( f.pointSizeF() - 4.0 );
            p.setFont( f );
            p.setOpacity( 0.75 );
            p.setPen( Qt::white );
            p.drawText( r.adjusted( 0, 2, -1, 1 ), Qt::AlignVCenter | Qt::AlignRight, QString::fromUtf8( "\xe2\x96\xbe " ) );
            p.setPen( isEnabled() ? Qt::black : Qt::gray );
            p.drawText( r.adjusted( 0, 1, -1, 0 ), Qt::AlignVCenter | Qt::AlignRight, QString::fromUtf8( "\xe2\x96\xbe " ) );
            p.setOpacity( 1.0 );
            p.setFont( m_action->font() );
            r = r.adjusted( 0, 0, -ArrowWidth, 0 );
        }
#endif

        if ( !icon().isNull() )
        {
            // if we have an icon, we show it
            p.drawPixmap( QRect( 3, 3, height() - 6, height() - 6 ), icon().pixmap( height() - 6, height() - 6, isEnabled() ? QIcon::Normal : QIcon::Disabled ) );
            r = r.adjusted( 16, 0, 0, 0 );
        }

        if ( !text().isEmpty() )
        {
            // print the text
            QFont f = m_action->font();
            f.setWeight( text().size() <= 1 ? QFont::Black : QFont::Normal );
            p.setFont( f );
            p.setOpacity( 0.95 );
            p.setPen( Qt::white );
            p.drawText( r.adjusted( 0, 0, 0, 0 ), Qt::AlignCenter, text() );
            p.setPen( isEnabled() ? Qt::black : Qt::gray );
            p.drawText( r.adjusted( 0, -1, 0, -1 ), Qt::AlignCenter, text() );
        }
    }
    else
#endif // Q_OS_MAC
    {
        QStylePainter p( this );
        QStyleOptionToolButton opt;
        initStyleOption( &opt );
        p.drawComplexControl( QStyle::CC_ToolButton, opt );
        return;
    }

}

/*!
  \class KDListViewToolBar KDListViewToolBar
  \ingroup gui macwidgets
  \brief Mac-style toolbar to be placed below list views.

  A KDListViewToolBar is a tool bar typically used to add or remove items
  in a list view on Mac OS X.  It can be used like a QToolBar, using
  addAction(), incl. QWidget::addAction() and QWidget::addActions().
*/

/*!
 \internal
*/
class KDListViewToolBar::Private
{
public:
    explicit Private( KDListViewToolBar* qq )
        : q( qq )
    {
    }

private:
    KDListViewToolBar* const q;

public:
    /*!
     Layouts the toolbar's buttons within it.
     \internal
    */
    void _kdt_layoutButtons()
    {
        int x = 0;
        Q_FOREACH( KDListViewToolButton * button, buttons ) {
            button->move( x, 0 );
            button->resize( button->sizeHint() );
            x += button->width();
        }
        q->updateGeometry();
    }
    
    void _kdt_triggered()
    {
        QAction* const action = static_cast<QAction*>( q->sender() );
        emit q->actionTriggered( action );
    }

    QList< KDListViewToolButton* > buttons;
};

/*!
 \signal KDListViewToolBar::actionTriggered
 This signal is emitted when an action in this toolbar is triggered. The parameter \a action holds the triggered action.
*/

/*!
 Creates a new KDListViewToolBar with parent \a parent.
*/
KDListViewToolBar::KDListViewToolBar( QWidget* parent )
    : QWidget( parent ),
      d( new Private( this ) )
{
    setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
}

/*!
 Destroys the KDListViewToolBar.
*/
KDListViewToolBar::~KDListViewToolBar()
{
}

bool KDListViewToolBar::event( QEvent* event )
{
    switch( event->type() )
    {
    case QEvent::LayoutRequest:
        {
            d->_kdt_layoutButtons();
            return true;
        }
    case QEvent::ActionAdded:
        {
            const QActionEvent * const e = static_cast<QActionEvent*>( event );
            if ( e->before() == 0 )
                d->buttons.push_back( new KDListViewToolButton( this, e->action() ) );
            else
                d->buttons.insert( actions().indexOf( e->before() ), new KDListViewToolButton( this, e->action() ) );
            d->_kdt_layoutButtons();
            connect( e->action(), SIGNAL(triggered()), this, SLOT(_kdt_triggered()));
            return true;
        }
    case QEvent::ActionRemoved:
        {
            const QActionEvent * const e = static_cast<QActionEvent*>( event );
            Q_ASSERT( actions().contains( e->action() ) );
            d->buttons[ actions().indexOf( e->action() ) ]->deleteLater();
            d->buttons.removeAt( actions().indexOf( e->action() ) );
            d->_kdt_layoutButtons();
            disconnect( e->action(), SIGNAL(triggered()), this, SLOT(_kdt_triggered()));
            return true;
        }
    default:
        return QWidget::event( event );
    }
}

QSize KDListViewToolBar::sizeHint() const
{
    return childrenRect().size();
}

/*!
 Creates a new action with the given \a text and adds it to the end of the toolbar.
 Returns the created action.
*/
QAction* KDListViewToolBar::addAction( const QString& text )
{
    QAction* const newAction = new QAction( text, this );
    QWidget::addAction( newAction );
    return newAction;
}

/*!
 Creates a new action with the given \a text and adds it to the end of the toolbar. The action's
 triggered() signal is connected to \a member in \a receiver.
 Returns the created action.
*/
QAction* KDListViewToolBar::addAction( const QString& text, const QObject* receiver, const char* member )
{
    QAction* const newAction = addAction( text );
    connect( newAction, SIGNAL(triggered()), receiver, member );
    return newAction;
}

/*!
 Creates a new separator action and adds it to the toolbar.
 Returns the created action.
*/
QAction* KDListViewToolBar::addSeparator()
{
    QAction* const newAction = new QAction( this );
    newAction->setSeparator( true );
    QWidget::addAction( newAction );
    return newAction;
}

/*!
 Returns the action at point \a p. This functions returns zero if no action was found.
*/
QAction* KDListViewToolBar::actionAt( const QPoint& p ) const
{
    KDListViewToolButton* const button = qobject_cast<KDListViewToolButton*>( childAt( p ) );
    return button == 0 ? 0 : button->m_action;
}

/*!
 \overload
 Returns the action at the point \a x, \a y. This function returns zero if no action was found.
*/
QAction* KDListViewToolBar::actionAt( int x, int y )
{
    return actionAt( QPoint( x, y ) );
}

/*!
 Inserts a separator into the toolbar in front of the toolbar item associated with the \a before action.
 \sa addSeparator
*/
QAction* KDListViewToolBar::insertSeparator( QAction* before )
{
    QAction* const newAction = new QAction( this );
    newAction->setSeparator( true );
    insertAction( before, newAction );
    return newAction;
}


#include "kdlistviewtoolbar.moc"
#include "moc_kdlistviewtoolbar.cpp"
