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

#include "kddoublespinbox.h"

#include <QCoreApplication>
#include <QEvent>
#include <QLineEdit>

namespace {
class SpinBoxPrivate
{
    friend class ::KDDoubleSpinBox;
    friend class ::KDSpinBox;
protected:
    SpinBoxPrivate()
        : showGroupSeparators( true )
    {

    }

protected:
    static QString fixGroupSeparators( const QString & text, const QString & prefix, const QString & suffix, const QLocale & loc );

protected:
    bool showGroupSeparators;
};
}

/*!
  Changes typed-in spaces to real group separators, if the locale's
  group separator is ' '.  Return the text w/o prefix and suffix.
  \internal
 */
QString SpinBoxPrivate::fixGroupSeparators( const QString & text, const QString & prefix, const QString & suffix, const QLocale & loc )
{
    QString t = text;
    const bool hasPrefix = t.startsWith( prefix );
    const bool hasSuffix = t.startsWith( suffix );
    if( hasPrefix )
        t = t.mid( prefix.length() );
    if( hasSuffix )
        t = t.mid( 0, t.length() - suffix.length() );

    // if group separator is space, accept ascii 32 as well as 160
    if( loc.groupSeparator() == QLatin1Char( '\xa0' ) )
        t.replace( QLatin1Char( '\x20' ), QLatin1Char( '\xa0' ) );
    
    return t;
}

/*!
  \class KDDoubleSpinBox
  \ingroup gui
  \brief A double spinbox with improved locale support.

  KDDoubleSpinBox shows double values as a spin box like
  QDoubleSpinBox does.  The only difference is that KDDoubleSpinBox
  (optionally) embeds \link QLocale::groupSeparator() group
  separators\endlink, which improves readability a lot.

  It also works around a bug in QDoubleSpinBox where the text isn't
  updated on locale changes.

  \sa KDSpinBox
*/

class KDDoubleSpinBox::Private : public SpinBoxPrivate {
public:
    explicit Private( KDDoubleSpinBox * )
        : SpinBoxPrivate() {}
};

/*!
 Creates a new KDDoubleSpinBox with \a parent.
 */
KDDoubleSpinBox::KDDoubleSpinBox( QWidget* parent )
    : QDoubleSpinBox( parent ),
      d( new Private( this ) )
{
}

/*!
 Destroys the KDDoubleSpinBox.
 */
KDDoubleSpinBox::~KDDoubleSpinBox()
{
}

/*!
  \property KDDoubleSpinBox::showGroupSeparators

  Specifies whether this KDDoubleSpinBox shows the value with \link
  QLocale::groupSeparator() group separators\endlink inserted. The
  default is to show them (showGroupSeparators = \c true).

  Get this property's value using %showGroupSeparators(), and set it
  using %setShowGroupSeparators().
*/
void KDDoubleSpinBox::setShowGroupSeparators( bool show )
{
    if ( show == d->showGroupSeparators )
        return;
    d->showGroupSeparators = show;
    QEvent e( QEvent::LocaleChange );
    QCoreApplication::sendEvent( this, &e );
}

bool KDDoubleSpinBox::showGroupSeparators() const
{
    return d->showGroupSeparators;
}

bool KDDoubleSpinBox::event( QEvent * e )
{
    if ( e->type() == QEvent::LocaleChange ) {
        // trigger a display-text update on LocaleChange
        // (QAbstractSpinBox doesn't react on that; this approach only
        // works because setSpecialValueText() doesn't short-cut
        // processing when the value hasn't changed)
        setSpecialValueText( specialValueText() );
    }
    return QDoubleSpinBox::event( e );
}

QString KDDoubleSpinBox::textFromValue( double value ) const
{
    if ( d->showGroupSeparators )
        return locale().toString( value, 'f', decimals() );
    else
        return QDoubleSpinBox::textFromValue( value );
}

double KDDoubleSpinBox::valueFromText( const QString& text ) const
{
    QString t;
    if ( d->showGroupSeparators ) {
        t = Private::fixGroupSeparators( text, prefix(), suffix(), locale() );
        t.remove( locale().groupSeparator() );
        t = prefix() + t + suffix();
    } else {
        t = text;
    }
    return QDoubleSpinBox::valueFromText( t );
}

QValidator::State KDDoubleSpinBox::validate( QString& text, int& pos ) const
{
   if ( !d->showGroupSeparators ||
        ( minimum() > -1000.0 && maximum() < 1000.0 ) ) // no group separators needed...
       return QDoubleSpinBox::validate( text, pos );

    const bool hasPrefix = text.startsWith( prefix() );
    QString t = Private::fixGroupSeparators( text, prefix(), suffix(), locale() );
    
    if( hasPrefix )
        pos -= prefix().length();

    // change the cursor pos, since we're removing the group separators for the check
    const int posDiff = t.left( pos ).count( locale().groupSeparator() );
    pos -= posDiff;

    t.remove( locale().groupSeparator() );

    if( hasPrefix )
        pos += prefix().length();

    t = prefix() + t + suffix();
    const QValidator::State state = QDoubleSpinBox::validate( t, pos );
    pos += posDiff;
    return state;
}

/*!
  \class KDSpinBox
  \ingroup gui
  \brief A spinbox with improved locale support.

  KDSpinBox shows double values as a spin box like
  QSpinBox does.  The only difference is that KDSpinBox
  (optionally) embeds \link QLocale::groupSeparator() group
  separators\endlink, which improves readability a lot.

  It also works around a bug in QSpinBox where the text isn't
  updated on locale changes.

  \sa KDDoubleSpinBox
*/

class KDSpinBox::Private : public SpinBoxPrivate {
public:
    explicit Private( KDSpinBox * )
        : SpinBoxPrivate() {}
};

/*!
 Creates a new KDSpinBox with \a parent.
 */
KDSpinBox::KDSpinBox( QWidget* parent )
    : QSpinBox( parent ),
      d( new Private( this ) )
{
}

/*!
 Destroys the KDSpinBox.
 */
KDSpinBox::~KDSpinBox()
{
}

/*!
  \property KDSpinBox::showGroupSeparators

  Specifies whether this KDSpinBox shows the value with \link
  QLocale::groupSeparator() group separators\endlink inserted. The
  default is to show them (showGroupSeparators = \c true).

  Get this property's value using %showGroupSeparators(), and set it
  using %setShowGroupSeparators().
*/
void KDSpinBox::setShowGroupSeparators( bool show )
{
    if ( show == d->showGroupSeparators )
        return;
    d->showGroupSeparators = show;
    QEvent e( QEvent::LocaleChange );
    QCoreApplication::sendEvent( this, &e );
}

bool KDSpinBox::showGroupSeparators() const
{
    return d->showGroupSeparators;
}

bool KDSpinBox::event( QEvent * e )
{
    if ( e->type() == QEvent::LocaleChange ) {
        // trigger a display-text update on LocaleChange
        // (QAbstractSpinBox doesn't react on that; this approach only
        // works because setSpecialValueText() doesn't short-cut
        // processing when the value hasn't changed)
        setSpecialValueText( specialValueText() );
    }
    return QSpinBox::event( e );
}

QString KDSpinBox::textFromValue( int value ) const
{
    if ( d->showGroupSeparators )
        return locale().toString( value );
    else
        return QSpinBox::textFromValue( value );
}

int KDSpinBox::valueFromText( const QString& text ) const
{
    QString t;
    if ( d->showGroupSeparators ) {
        t = Private::fixGroupSeparators( text, prefix(), suffix(), locale() );
        t.remove( locale().groupSeparator() );
        t = prefix() + t + suffix();
    } else {
        t = text;
    }
    return QSpinBox::valueFromText( t );
}

QValidator::State KDSpinBox::validate( QString& text, int& pos ) const
{
   if ( !d->showGroupSeparators ||
        ( minimum() > -1000.0 && maximum() < 1000.0 ) ) // no group separators needed...
       return QSpinBox::validate( text, pos );

    const bool hasPrefix = text.startsWith( prefix() );
    QString t = Private::fixGroupSeparators( text, prefix(), suffix(), locale() );
    
    if( hasPrefix )
        pos -= prefix().length();

    // change the cursor pos, since we're removing the group separators for the check
    const int posDiff = t.left( pos ).count( locale().groupSeparator() );
    pos -= posDiff;

    t.remove( locale().groupSeparator() );

    if( hasPrefix )
        pos += prefix().length();

    t = prefix() + t + suffix();
    const QValidator::State state = QSpinBox::validate( t, pos );
    pos += posDiff;
    return state;
}

#include "moc_kddoublespinbox.cpp"
