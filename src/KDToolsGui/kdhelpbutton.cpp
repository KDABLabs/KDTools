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

#include "kdhelpbutton.h"

#include "macwidget_p.h"

#include <QApplication>
#include <QPainter>
#include <QResizeEvent>
#include <QStyleOptionButton>
#include <QStylePainter>

#ifdef Q_OS_MAC
#include <QMacCocoaViewContainer>
extern QMacCocoaViewContainer* createNSHelpButton( QWidget* parent ); // implemented in kdhelpbutton_mac.mm
#endif

using namespace _macwidget;

class KDHelpButton::Private
{
public:
    explicit Private( KDHelpButton * qq )
#ifdef Q_OS_MAC
        : button( createNSHelpButton( qq ) )
#endif
    {
        Q_UNUSED( qq );
        // we can't use sizeHint here yet, therefore we post a StyleChange event
        qApp->postEvent( qq, new QEvent( QEvent::StyleChange ) );
    }
#ifdef Q_OS_MAC
    QMacCocoaViewContainer* const button;
#endif
};

/*!
  \class KDHelpButton KDHelpButton
  \ingroup gui macwidgets
  \brief A Mac-style help button

  A KDHelpButton is just a push button with its text property
  pre-defined to "?". If you use this button with QMacStyle, it
  looks like the help button used by Mac OS. The
  QAbstractButton::text property is ignored.
*/

/*!
 Creates a new KDHelpButton with parent \a parent.
 */
KDHelpButton::KDHelpButton( QWidget* parent )
    : QAbstractButton( parent ),
      d( new Private( this ) )
{
}

/*!
 Destroys the HelpButton
 */
KDHelpButton::~KDHelpButton()
{
}

/*!
    Initialize \a option with the values from this QPushButton. This method is useful
    for subclasses when they need a QStyleOptionButton, but don't want to fill
    in all the information themselves.

    \sa QStyleOption::initFrom()
*/
void KDHelpButton::initStyleOption( QStyleOptionButton* option ) const
{
    if (!option)
        return;

    option->initFrom(this);
    option->features = QStyleOptionButton::None;
    if (isDown())
        option->state |= QStyle::State_Sunken;
    if (!isDown())
        option->state |= QStyle::State_Raised;
    option->text = tr( "?" );
}


void KDHelpButton::changeEvent( QEvent* event )
{
    if( event->type() == QEvent::StyleChange )
    {
#ifdef Q_OS_MAC
        if( isMacStyle( style() ) )
            setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
        else
#endif
            setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );  // unset constraints
        updateGeometry();
    }
    QAbstractButton::changeEvent( event );
}

QSize KDHelpButton::minimumSizeHint() const
{
    return sizeHint();
}

QSize KDHelpButton::sizeHint() const
{
#ifdef Q_OS_MAC
    if( isMacStyle( style() ) ) {
        return d->button->sizeHint();
    } else
#endif
    {
        ensurePolished();

        QStyleOptionButton opt;
        initStyleOption(&opt);

        // calculate contents size...
        QSize size = opt.fontMetrics.size(Qt::TextShowMnemonic, tr( "?" ) );
        opt.rect.setSize(size); // PM_MenuButtonIndicator depends on the height
    
        size = (style()->sizeFromContents(QStyle::CT_PushButton, &opt, size, this).expandedTo(QApplication::globalStrut()));
        size.setWidth(size.height());
        return size;
    }
}

void KDHelpButton::mousePressEvent( QMouseEvent* event )
{
#ifdef Q_OS_MAC
    if( isMacStyle( style() ) ) {
       event->ignore();
       return;
    }
#endif
    QAbstractButton::mousePressEvent( event );
}

void KDHelpButton::resizeEvent( QResizeEvent* event )
{
#ifdef Q_OS_MAC
    d->button->resize( event->size() );
#else
    Q_UNUSED( event )
#endif
}

void KDHelpButton::paintEvent( QPaintEvent* event )
{
    Q_UNUSED( event )
#ifdef Q_OS_MAC
    const bool mac = isMacStyle( style() );
    d->button->setVisible( mac );
    if( !mac )
#endif
    {
        QStylePainter p(this);
        QStyleOptionButton option;
        initStyleOption(&option);
        p.drawControl(QStyle::CE_PushButton, option);
    }
}
