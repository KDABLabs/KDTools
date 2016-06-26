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

#include "kdproperty.h"
#include "kdpropertymodel.h"

#include <QApplication>
#include <QStyleOption>

#include <float.h>
#include <limits.h>

#define d d_func()

//
//
// KDPropertyInterface
//
//


/*!\class KDPropertyInterface KDProperty
  \ingroup KDPropertyView
  \brief This is the interface implemented by all items
  that can be put into a KDPropertyModel.

  To create new properties, don't subclass this class,
  subclass KDAbstractProperty<T> instead.
*/

class KDPropertyInterface::Private {
    friend class ::KDPropertyInterface;
public:
    Private();
    Private( const Private & other );
    ~Private();

private:
    KDPropertyGroupInterface* parent;
    KDPropertyModel* model;
    QString helptext;
    bool reset      : 1;
    bool standalone : 1;
    bool changed    : 1;
};

KDPropertyInterface::Private::Private()
  : parent( 0 ),
    model( 0 ),
    reset( false ),
    standalone( true ),
    changed( false )
{

}

KDPropertyInterface::Private::Private( const Private & other )
    : parent( 0 ),
      model( 0 ),
      helptext( other.helptext ),
      reset( other.reset ),
      standalone( other.standalone ),
      changed( other.changed )
{

}

KDPropertyInterface::Private::~Private() {}


/*!
  Constructor. Creates a KDPropertyInterface with no parent.
 */
KDPropertyInterface::KDPropertyInterface()
    : _d( new Private )
{
    init( false );
}

KDPropertyInterface::KDPropertyInterface( const KDPropertyInterface & other )
    : _d( new Private( *other.d ) )
{
    init( true );
}

/*! Destructor. */
KDPropertyInterface::~KDPropertyInterface()
{
    delete _d; _d = 0;
}

void KDPropertyInterface::init( bool ) {}

/*! Sets the parent property to \a p */
void KDPropertyInterface::setParent(KDPropertyGroupInterface* p)
{
  d->parent = p;
}

/*! \returns the parent property */
KDPropertyGroupInterface * KDPropertyInterface::parent() const
{
    return d->parent;
}

/*! Sets the model that this property is part of to \a model */
void KDPropertyInterface::setModel( KDPropertyModel* model)
{
  d->model = model;
}

/*! \return the model that this property is part of */
KDPropertyModel * KDPropertyInterface::model() const
{
    return d->model;
}

/*! If \a c is true, this method marks the value of this
  property as being changed from the initial value. For
  properties that are part of a composite property, the value
  is propagated to the composite ancestor.

  Changed properties are displayed in boldface in the view.
*/
void KDPropertyInterface::setChanged(bool c)
{
  if( !isStandalone() ) {
    KDPropertyInterface* prop = parent();
    while( prop && !prop->isStandalone() ) prop = prop->parent();
    if( prop ) prop->setChanged(c);
  }
  d->changed = c;
}

/*! \returns whether this property was changed */
bool KDPropertyInterface::isChanged() const {
    return d->changed;
}

/*! If \a r is true, the property is marked as having
  "reset" functionality. This is currently not used by the view.
  \todo Evaluate if we need this.
*/
void KDPropertyInterface::setHasReset(bool r)
{
    d->reset = r;
}

/*!
  \returns whether the property is marked as having "reset"
  functionality.
*/
bool KDPropertyInterface::hasReset() const
{
    return d->reset;
}

/*! If \a s is true, the property is marked as being a standalone
  property. This is the default. Properties that are not standalone
  are part of a composite property.
*/
void KDPropertyInterface::setStandalone(bool s)
{
    d->standalone = s;
}

/*! \return whether the property is marked as being standalone. */
bool KDPropertyInterface::isStandalone() const
{
    return d->standalone;
}

/*! Call this if the internal state of the property
  changes. It will notify the model
*/
void KDPropertyInterface::update()
{
  //qDebug() << "KDPropertyInterface::update(), model="<<model();
  if ( model() ) model()->update(this);
}


/*!\fn KDPropertyInterface::name() const
  Returns the display name of this property.
*/

/*!
  Set an additional help text on this property.
*/
void KDPropertyInterface::setHelpText( const QString & txt )
{
    d->helptext = txt;
}

/*!
  Returns the extra helptext associated with this
  property or QString::null is there is none.
*/
QString KDPropertyInterface::helpText() const
{
    return d->helptext;
}

/*!\fn KDPropertyInterface::setValue(const QVariant& v)
  Set the value of this property to \a v.
*/
/*!\fn KDPropertyInterface::value() const
  Returns the value of this property.
*/
/*!\fn KDPropertyInterface::toString() const
  Returns the display text for this property.
 */

/*!
  Returns the decoration for this property. This
  can typically be an icon.

  The default implementation returns nothing.
*/
QVariant KDPropertyInterface::decoration() const {
    return QVariant();
}

/*!\fn KDPropertyInterface::isContainer() const
  Returns true is the property is a container that
  can hold other properties. This is the case for
  KDAbstractCompositeProperty and KDPropertyCategory.
*/

/*!\fn KDPropertyInterface::isEditable() const
  Returns true for editable properties, and false
  for readonly properties.
*/

/*!\fn KDPropertyInterface::editorType() const
  Returns QVariant() if the property uses the
  standardeditors or a QVariant with a QString containing
  the classname of a special editor.

  The default implementation return QVariant().

  \see KDPropertyEditorFactory
  \see KDPropertyViewItemDelegate
*/
QVariant KDPropertyInterface::editorType() const {
    return QVariant(); // default editor
}

/*! \todo undocumented */
QVariant KDPropertyInterface::editorHint( int ) const
{
    return QVariant();
}

//
//
// KDPropertyGroupInterface
//
//


/*!\class KDPropertyGroupInterface KDProperty
  \ingroup KDPropertyView
  \brief This is the interface implemented by all
  KDPropertyInterface items that can contain other
  KDPropertyInterface items.
*/

class KDPropertyGroupInterface::Private : public KDPropertyInterface::Private{
    friend class ::KDPropertyGroupInterface;
public:

};

KDPropertyGroupInterface::KDPropertyGroupInterface( const KDPropertyGroupInterface & other )
    : KDPropertyInterface( new Private( *other.d ), true )
{
    init( true );
}

KDPropertyGroupInterface::KDPropertyGroupInterface()
    : KDPropertyInterface( new Private, false )
{
    init( false );
}

KDPropertyGroupInterface::~KDPropertyGroupInterface() {}

void KDPropertyGroupInterface::init( bool ) {}

/*! Reimplmentation of KDPropertyInterface::setModel() that
  sets the model on child properties too.
*/
void KDPropertyGroupInterface::setModel( KDPropertyModel* model )
{
  KDPropertyInterface::setModel(model);
  for( int i = 0; i < propertyCount(); ++i ) {
    propertyAt(i)->setModel(model);
  }
}


//
//
// KDAbstractPropertyBase
//
//

class KDAbstractPropertyBase::Private : public  KDPropertyInterface::Private {
    friend class ::KDAbstractPropertyBase;
public:
    explicit Private( const QVariant & value, const QString & name );
    ~Private();

protected:
    QVariant value;
    QString name;
    bool enabled;
};

KDAbstractPropertyBase::Private::Private( const QVariant & value_, const QString & name_ )
    : KDPropertyInterface::Private(),
      value( value_ ),
      name( name_ ),
      enabled( true )
{

}

KDAbstractPropertyBase::Private::~Private() {}

KDAbstractPropertyBase::KDAbstractPropertyBase( const KDAbstractPropertyBase & other )
    : KDPropertyInterface( new Private( *other.d ), true )
{
    init( true );
}

KDAbstractPropertyBase::KDAbstractPropertyBase( const QVariant & val, const QString & name )
    : KDPropertyInterface( new Private( val, name ), false )
{
    init( false );
}

KDAbstractPropertyBase::~KDAbstractPropertyBase() {}

void KDAbstractPropertyBase::init( bool ) {}

void KDAbstractPropertyBase::setValue( const QVariant & val ) {
    if ( val == d->value )
        return;
    d->value = val;
    update();
}

QVariant KDAbstractPropertyBase::value() const {
    return d->value;
}

QString KDAbstractPropertyBase::name() const {
    return d->name;
}

bool KDAbstractPropertyBase::isEditable() const {
    return true;
}

bool KDAbstractPropertyBase::isContainer() const {
    return false;
}

bool KDAbstractPropertyBase::isCategory() const {
    return false;
}

void KDAbstractPropertyBase::setEnabled( bool on ) {
    if ( on == d->enabled )
        return;
    d->enabled = on;
    update();
}

bool KDAbstractPropertyBase::isEnabled() const {
    if ( parent() && parent()->isCategory() && !parent()->isEnabled() )
        return false;
    else
        return d->enabled;
}


//
//
// KDAbstractProperty
//
//

/*!\class KDAbstractProperty KDProperty
  \brief This template is the baseclass of all simple properties.

  Subclass to implement specific simple-valued properties.
*/

/*!\var KDAbstractProperty::m_value
  Holds the value of this KDAbstractProperty.
*/


//
//
// KDAbstractCompositeProperty
//
//

/*!\class KDAbstractCompositeProperty KDProperty
  \ingroup KDPropertyView
  \brief This is the baseclass of all composite
  properties. A composite property is
  made from one or properties which values
  make up the value of the composite property.

  For example a property holding a QSize value is
  is composed by two simple properties holding integer
  values.
*/

class KDAbstractCompositeProperty::Private : public KDPropertyGroupInterface::Private {
    friend class ::KDAbstractCompositeProperty;
public:
    Private( const Private & other );
    explicit Private( const QString & name );
    ~Private();

private:
    QString name;
    bool enabled;
    QList<KDPropertyInterface*> properties;
};

KDAbstractCompositeProperty::Private::Private( const Private & other )
    : KDPropertyGroupInterface::Private( other ),
      name( other.name ),
      enabled( other.enabled ),
      properties()
{
    Q_FOREACH( KDPropertyInterface * i, other.properties )
        if ( i )
            properties.push_back( i->clone() );
}


KDAbstractCompositeProperty::Private::Private( const QString & name )
    : KDPropertyGroupInterface::Private(),
      name( name ),
      enabled( true ),
      properties()
{

}

KDAbstractCompositeProperty::Private::~Private() {}

KDAbstractCompositeProperty::KDAbstractCompositeProperty( const KDAbstractCompositeProperty & other )
    : KDPropertyGroupInterface( new Private( *other.d ), true )
{
    init( true );
}

KDAbstractCompositeProperty::KDAbstractCompositeProperty( const QString & name )
    : KDPropertyGroupInterface( new Private( name ), false )
{
    init( false );
}

KDAbstractCompositeProperty::~KDAbstractCompositeProperty()
{
    qDeleteAll( d->properties );
}

void KDAbstractCompositeProperty::init( bool ) {}

QString KDAbstractCompositeProperty::name() const
{
    return d->name;
}

bool KDAbstractCompositeProperty::isContainer() const
{
    return true;
}

bool KDAbstractCompositeProperty::isCategory() const
{
    return false;
}

KDPropertyInterface* KDAbstractCompositeProperty::propertyAt(int index) const
{
  if( index >= 0 && index < d->properties.size() )
    return d->properties.at(index);
  else return 0;
}

int KDAbstractCompositeProperty::indexOf( KDPropertyInterface * prop ) const
{
    return d->properties.indexOf( prop );
}

int KDAbstractCompositeProperty::propertyCount() const
{
    return d->properties.size();
}

/*! Add a property to this composite. */
void KDAbstractCompositeProperty::addProperty( KDPropertyInterface * prop )
{
    if ( !prop )
        return;
    prop->setParent( this );
    prop->setModel( model() );
    d->properties.push_back( prop );
}

#if 0
void KDAbstractCompositeProperty::setChanged(bool c)
{
  Q_FOREACH( KDPropertyInterface* i, d->properties ) {
    i->setChanged(c);
  }
}

bool KDAbstractCompositeProperty::changed() const
{
  Q_FOREACH( KDPropertyInterface* i, d->properties ) {
    if( i->changed() ) return true;
  }
  return false;
}
#endif

void KDAbstractCompositeProperty::setEnabled(bool e)
{
  Q_FOREACH( KDPropertyInterface* i, d->properties ) {
    i->setEnabled(e);
  }
  d->enabled = e;
  update();
}

bool KDAbstractCompositeProperty::isEnabled() const
{
  if( parent() && parent()->isCategory() && !parent()->isEnabled() ) return false;
  else return d->enabled;
}

//
//
// KDPropertyCategory
//
//

/*!\class KDPropertyCategory KDProperty
  \ingroup KDPropertyView
  \brief This class represents a category of properties.
*/

class KDPropertyCategory::Private : public KDPropertyGroupInterface::Private {
    friend class ::KDPropertyCategory;
public:
    Private( const Private & other );
    Private( const QString & name, const QString & info );
    ~Private();

private:
    QString name;
    QString info;
    QIcon icon;
    Qt::CheckState checked;
    bool checkable;
    bool enabled;
    QList<KDPropertyInterface*> properties;

};

KDPropertyCategory::Private::Private( const Private & other )
    : KDPropertyGroupInterface::Private( other ),
      name( other.name ),
      info( other.info ),
      icon( other.icon ),
      checked( other.checked ),
      checkable( other.checkable ),
      enabled( other.enabled ),
      properties()
{
    Q_FOREACH( KDPropertyInterface * i, other.properties )
        if ( i )
            properties.push_back( i->clone() );
}

KDPropertyCategory::Private::Private( const QString & name_, const QString & info_ )
    : KDPropertyGroupInterface::Private(),
      name( name_ ),
      info( info_ ),
      icon(),
      checked( Qt::PartiallyChecked ),
      checkable( true ),
      enabled( true ),
      properties()
{

}

KDPropertyCategory::Private::~Private() {}

KDPropertyCategory::KDPropertyCategory( const KDPropertyCategory & other )
    : KDPropertyGroupInterface( new Private( *other.d ), true )
{
    init( true );
}

/*!
  Constructor. Creates an instance of a property category with name \a name
  and optional extra info-text \a info
*/
KDPropertyCategory::KDPropertyCategory( const QString & name, const QString & info )
    : KDPropertyGroupInterface( new Private( name, info ), false )
{
    init( false );
}

/*!
  Destroys the category and all properties contained
  within it.
*/
KDPropertyCategory::~KDPropertyCategory()
{
    qDeleteAll( d->properties );
}

void KDPropertyCategory::init( bool ) {}


QString KDPropertyCategory::name() const
{
    return d->name;
}

bool KDPropertyCategory::isContainer() const
{
    return true;
}

bool KDPropertyCategory::isCategory() const
{
    return true;
}

void KDPropertyCategory::setValue( const QVariant & )
{

}

QVariant KDPropertyCategory::value() const
{
    return QVariant();
}

QIcon KDPropertyCategory::icon() const
{
    return d->icon;
}


void KDPropertyCategory::setIcon( const QIcon & ic )
{
    d->icon = ic;
    update();
}

QString KDPropertyCategory::toString() const
{
    return d->info;
}

QVariant KDPropertyCategory::decoration() const
{
    return d->icon;
}

void KDPropertyCategory::setChecked(bool e)
{
  if( e ) d->checked = Qt::Checked;
  else d->checked = Qt::Unchecked;

  setEnabled(e);
}

Qt::CheckState KDPropertyCategory::isChecked() const
{
  if( d->checked != Qt::PartiallyChecked ) return d->checked;
  KDPropertyGroupInterface* p = parent();
  if( p && p->isCategory() && p->parent() != 0 ) {
    if( static_cast<KDPropertyCategory*>(p)->isChecked() ) {
      return Qt::PartiallyChecked;
    } else {
      return Qt::Unchecked;
    }
  } else {
    return (d->checked==Qt::Unchecked)?Qt::Unchecked:Qt::Checked;
  }
}

/*! This method controls whether the category
  displays a checkbox or not to enable/disable
  the category. Defaults to true.
*/
void KDPropertyCategory::setCheckable(bool c)
{
    if ( c == d->checkable )
        return;
    d->checkable = c;
    update();
}

/*! \returns true is this category has
  a checkbox and false otherwise.
*/
bool KDPropertyCategory::isCheckable() const
{
  return d->checkable;
}

void KDPropertyCategory::setEnabled(bool e)
{
    if ( e == d->enabled )
        return;
    d->enabled = e;
    update();
    Q_FOREACH( KDPropertyInterface * i, d->properties )
        i->update();
}

bool KDPropertyCategory::isEnabled() const
{
  if( parent() && parent()->isCategory() && !parent()->isEnabled() ) return false;
  else return d->enabled;
}

int KDPropertyCategory::indexOf( KDPropertyInterface * property ) const
{
    return d->properties.indexOf( property );
}

int KDPropertyCategory::propertyCount() const
{
    return d->properties.size();
}

KDPropertyInterface* KDPropertyCategory::propertyAt(int index) const
{
  if( index >= 0 && index < d->properties.size() )
    return d->properties.at(index);
  else return 0;
}

bool KDPropertyCategory::isEditable() const
{
    return false;
}

/*! Add a property to this category. \a property must be allocated
  on the heap and the category becomes the owner of \a property
*/
void KDPropertyCategory::addProperty(KDPropertyInterface* property)
{
  property->setParent(this);
  property->setModel(model());
  d->properties.push_back(property);
}

/*! Release ownership of a property from a KDPropertyCategory.
  Ownership is transferred back to the caller, who is responsible
  for deleting it
*/
void KDPropertyCategory::removeProperty(KDPropertyInterface* property)
{
  property->setModel(0);
  property->setParent(0);
  d->properties.removeAll(property);
}

KDPropertyCategory * KDPropertyCategory::clone() const
{
    return new KDPropertyCategory( *this );
}

//
//
// KDStringProperty
//
//

/*!\class KDStringProperty KDProperty
  \ingroup KDPropertyView
  \brief A property with a QString value.
*/

class KDStringProperty::Private : public KDAbstractPropertyBase::Private {
    friend class ::KDStringProperty;
public:
    Private( const QString & mask, const QString & value, const QString & name );
    ~Private();

private:
    QString mask;
    bool password;
};

KDStringProperty::Private::Private( const QString & mask_, const QString & value_, const QString & name_ )
    : KDAbstractPropertyBase::Private( value_, name_ ),
      mask( mask_ ),
      password( false )
{

}

KDStringProperty::Private::~Private() {}

KDStringProperty * KDStringProperty::clone() const
{
    return new KDStringProperty( *this );
}

KDStringProperty::KDStringProperty( const KDStringProperty & other )
    : KDAbstractProperty<QString>( new Private( *other.d ), true )
{
    init( true );
}

/*! Constructor. Creates a string property with name \a name
  and value \a value.
*/
KDStringProperty::KDStringProperty(const QString &value, const QString &name)
    : KDAbstractProperty<QString>( new Private( QString(), value, name ), false )
{
    init( false );
}

/*! Constructor. Creates a string property with name \a name,
  value \a value and input mask \a mask. The input mask is used as a hint
  for the editor.
*/
KDStringProperty::KDStringProperty(const QString& mask, const QString &value, const QString &name)
    : KDAbstractProperty<QString>( new Private( mask, value, name ), false )
{
    init( false );
}

KDStringProperty::~KDStringProperty() {}

void KDStringProperty::init( bool ) {}

/*! If set to true, the string will not be displayed on screen. Instead a
  number of '*' characters will be displayed. The editor used to edit the
  property is hinted about this too.
*/
void KDStringProperty::setPasswordMode(bool m)
{
    d->password = m;
}

bool KDStringProperty::isPasswordMode() const {
    return d->password;
}

/*! Set the inputmask of the property. This is used as a hint for the editor.
  Setting \a m to QString::null will disable the input mask.
*/
void KDStringProperty::setInputMask(const QString& m)
{
    d->mask = m;
}

QString KDStringProperty::inputMask() const {
    return d->mask;
}

QVariant KDStringProperty::editorType() const
{
    return QString::fromLatin1("KDStringPropertyEditor");
}

QVariant KDStringProperty::editorHint(int role) const
{
  if( role == KDPropertyModel::InputMaskHint ) return qVariantFromValue( inputMask() );
  else if( role == KDPropertyModel::PasswordModeHint ) return qVariantFromValue( isPasswordMode() );
  else return QVariant();
}

QString KDStringProperty::toString() const
{
  QString res = data();
  if ( isPasswordMode() ) {
    QStyleOption opt;
    return res.fill(QApplication::style()->styleHint(QStyle::SH_LineEdit_PasswordCharacter, &opt, 0));
  }
  return res;
}


//
//
// KDComboProperty
//
//



/*!\class KDComboProperty KDProperty
  \ingroup KDPropertyView
  \brief A property with a QString value in an editor
  that is a combobox with a list of choices.
*/

class KDComboProperty::Private : public KDAbstractPropertyBase::Private {
    friend class ::KDComboProperty;
public:
    Private( const QStringList & choices_, const QString & value_, const QString & name_ );
    ~Private();

private:
    QStringList choices;
    bool editable;
};

KDComboProperty::Private::Private( const QStringList & choices_, const QString & value_, const QString & name_ )
    : KDAbstractPropertyBase::Private( value_, name_ ),
      choices( choices_ ),
      editable( true )
{

}

KDComboProperty::Private::~Private() {}

KDComboProperty::KDComboProperty( const KDComboProperty & other )
    : KDAbstractProperty<QString>( new Private( *other.d ), true )
{
    init( true );
}

KDComboProperty * KDComboProperty::clone() const
{
    return new KDComboProperty( *this );
}

/*! Constructor. Creates a property with value \a value, name \a name
  and \a choices as the list of choices for the combobox. By default the
  combobox is editable.
*/
KDComboProperty::KDComboProperty( const QStringList& choices, const QString& value, const QString& name )
    : KDAbstractProperty<QString>( new Private( choices, value, name ), false )
{
    init( false );
}

KDComboProperty::~KDComboProperty() {}

void KDComboProperty::init( bool ) {}

/*! Sets the list of choices for the combobox to \a choices */
void KDComboProperty::setChoices( const QStringList& choices )
{
    d->choices = choices;
}

QStringList KDComboProperty::choices() const
{
    return d->choices;
}

void KDComboProperty::setEditable( bool e )
{
    d->editable = e;
}

bool KDComboProperty::isEditable() const
{
    return d->editable;
}

/*! \returns the value of this property */
QString KDComboProperty::toString() const
{
    return data();
}

QVariant KDComboProperty::editorType() const
{
    return QString::fromLatin1("KDComboPropertyEditor");
}

QVariant KDComboProperty::editorHint(int hint) const
{
  if( hint == KDPropertyModel::ChoiceOptionsHint ) {
    return qVariantFromValue(choices());
  } else if( hint == KDPropertyModel::EditableHint ) {
    return isEditable();
  } else {
    return QVariant();
  }
}


//
//
// KDFilenameProperty
//
//

class KDFilenameProperty::Private : public KDAbstractPropertyBase::Private {
    friend class ::KDFilenameProperty;
public:
    Private( const QString & value, const QString & name )
        : KDAbstractPropertyBase::Private( value, name ) {}
    ~Private() {}
};

/*!\class KDFilenameProperty KDProperty
  \ingroup KDPropertyView
  \brief A property that holds a filename
*/

KDFilenameProperty::KDFilenameProperty( const KDFilenameProperty & other )
    : KDAbstractProperty<QString>( new Private( *other.d ), true )
{
    init( true );
}

KDFilenameProperty * KDFilenameProperty::clone() const
{
    return new KDFilenameProperty( *this );
}


/*! Constructor. Creates a filename property with name \a name
  and value \a value.
*/
KDFilenameProperty::KDFilenameProperty(const QString &value, const QString &name)
    : KDAbstractProperty<QString>( new Private( value, name ), false )
{
    init( false );
}

KDFilenameProperty::~KDFilenameProperty() {}

void KDFilenameProperty::init( bool ) {}

QString KDFilenameProperty::toString() const
{
  return data();
}

QVariant KDFilenameProperty::editorType() const
{
    return QString::fromLatin1("KDFilenamePropertyEditor");
}


//
//
// KDBooleanProperty
//
//


/*!\class KDBooleanProperty KDProperty
  \ingroup KDPropertyView
  \brief A property with an boolean value.
*/

class KDBooleanProperty::Private : public KDAbstractPropertyBase::Private {
    friend class ::KDBooleanProperty;
public:
    Private( bool value_, const QString & name_, const QString & infotext_ )
        : KDAbstractPropertyBase::Private( value_, name_ ), infotext( infotext_ ) {}
    ~Private() {}

private:
    QString infotext;
};

KDBooleanProperty::KDBooleanProperty( const KDBooleanProperty & other )
    : KDAbstractProperty<bool>( new Private( *other.d ), true )
{
    init( true );
}

KDBooleanProperty * KDBooleanProperty::clone() const
{
    return new KDBooleanProperty( *this );
}

/*! Constructor. Creates a boolean property with name \a name
  and value \a value. In the view, it will display as a checkbox. The
  (optional) checkbox label text is set to \a infotext.
*/
KDBooleanProperty::KDBooleanProperty(bool value_, const QString &name_, const QString& infotext_)
    : KDAbstractProperty<bool>( new Private( value_, name_, infotext_ ), false )
{
    init( false );
}

KDBooleanProperty::~KDBooleanProperty() {}

void KDBooleanProperty::init( bool ) {}

QString KDBooleanProperty::toString() const
{
    return d->infotext;
}

QVariant KDBooleanProperty::editorType() const
{
    return QString::fromLatin1("KDBooleanPropertyEditor");
}

void KDBooleanProperty::setInfoText( const QString & it )
{
    d->infotext = it;
}

QString KDBooleanProperty::infoText() const
{
    return d->infotext;
}

//
//
// KDIntProperty
//
//


/*!\class KDIntProperty KDProperty
  \ingroup KDPropertyView
  \brief A property with an integer value.
*/

class KDIntProperty::Private : public KDAbstractPropertyBase::Private {
    friend class ::KDIntProperty;
public:
    Private( int min, int max, int value, const QString & name )
        : KDAbstractPropertyBase::Private( value, name ), minimum( min ), maximum( max ) {}
    ~Private() {}

private:
    int minimum;
    int maximum;
};

KDIntProperty::KDIntProperty( const KDIntProperty & other )
    : KDAbstractProperty<int>( new Private( *other.d ), true )
{
    init( true );
}

KDIntProperty * KDIntProperty::clone() const
{
    return new KDIntProperty( *this );
}

KDIntProperty::KDIntProperty(int value, const QString &name)
    : KDAbstractProperty<int>( new Private( INT_MIN, INT_MAX, value, name ), false )
{
    init( false );
}

KDIntProperty::KDIntProperty(int min, int max, int value, const QString &name)
    : KDAbstractProperty<int>( new Private( min, max, value, name ), false )
{
    init( false );
}

KDIntProperty::~KDIntProperty() {}

void KDIntProperty::init( bool ) {}

QString KDIntProperty::toString() const { return QString::number(data()); }

QVariant KDIntProperty::editorType() const
{
    return QLatin1String("KDIntPropertyEditor");
}

QVariant KDIntProperty::editorHint(int hint) const
{
  switch (hint) {
  case KDPropertyModel::MinimumHint: return d->minimum;
  case KDPropertyModel::MaximumHint: return d->maximum;
  default: return QVariant();
  }
}

//
//
// KDUIntProperty
//
//


/*!\class KDUIntProperty KDProperty
  \ingroup KDPropertyView
  \brief A property with an unsigned integer value.
*/

class KDUIntProperty::Private : public KDAbstractPropertyBase::Private {
    friend class ::KDUIntProperty;
public:
    Private( uint value, const QString & name )
        : KDAbstractPropertyBase::Private( value, name ) {}
    ~Private() {}
};

KDUIntProperty::KDUIntProperty( const KDUIntProperty & other )
    : KDAbstractProperty<uint>( new Private( *other.d ), true )
{
    init( true );
}

KDUIntProperty * KDUIntProperty::clone() const
{
    return new KDUIntProperty( *this );
}

KDUIntProperty::KDUIntProperty(uint value, const QString &name)
    : KDAbstractProperty<uint>( new Private( value, name ), false )
{
    init( false );
}

KDUIntProperty::~KDUIntProperty() {}

void KDUIntProperty::init( bool ) {}

QString KDUIntProperty::toString() const { return QString::number(data()); }

//
//
// KDDoubleProperty
//
//

class KDDoubleProperty::Private : public KDAbstractPropertyBase::Private {
    friend class ::KDDoubleProperty;
public:
    Private( double min, double max, double value, const QString & name )
      : KDAbstractPropertyBase::Private( value, name ), minimum(min), maximum(max) {}
    ~Private() {}
private:
    double minimum;
    double maximum;
};

/*!\class KDDoubleProperty KDProperty
  \ingroup KDPropertyView
  \brief A property with a double value.
*/

KDDoubleProperty::KDDoubleProperty( const KDDoubleProperty & other )
    : KDAbstractProperty<double>( new Private( *other.d ), true )
{
    init( true );
}

KDDoubleProperty * KDDoubleProperty::clone() const
{
    return new KDDoubleProperty( *this );
}

/*! Constructor. Creates a double property with name \a name
  and value \a value.
*/
KDDoubleProperty::KDDoubleProperty(double value, const QString &name)
    : KDAbstractProperty<double>( new Private( DBL_MIN, DBL_MAX, value, name ), false )
{
    init( false );
}

KDDoubleProperty::KDDoubleProperty( double min, double max, double value, const QString & name )
  : KDAbstractProperty<double>( new Private( min, max, value, name ), false )
{
  init( false );
}

KDDoubleProperty::~KDDoubleProperty() {}

void KDDoubleProperty::init( bool ) {}

QString KDDoubleProperty::toString() const
{
  return QString::number(data());
}

QVariant KDDoubleProperty::editorType() const
{
    return QLatin1String("KDDoublePropertyEditor");
}

QVariant KDDoubleProperty::editorHint(int hint) const
{
  switch (hint) {
  case KDPropertyModel::MinimumHint: return d->minimum;
  case KDPropertyModel::MaximumHint: return d->maximum;
  default: return QVariant();
  }
}

//
//
// KDDateProperty
//
//

/*!\class KDDateProperty KDProperty
  \ingroup KDPropertyView
  \brief A property with a QDate value.
*/

class KDDateProperty::Private : public KDAbstractPropertyBase::Private {
    friend class ::KDDateProperty;
public:
    Private( const QDate & value, const QString & name )
        : KDAbstractPropertyBase::Private( value, name ) {}
    ~Private() {}
};

KDDateProperty::KDDateProperty( const KDDateProperty & other )
    : KDAbstractProperty<QDate>( new Private( *other.d ), true )
{
    init( true );
}


KDDateProperty * KDDateProperty::clone() const
{
    return new KDDateProperty( *this );
}

/*! Constructor. Creates a QDate property with name \a name
  and value \a value.
*/
KDDateProperty::KDDateProperty(const QDate& value, const QString &name)
    : KDAbstractProperty<QDate>( new Private( value, name ), false )
{
    init( false );
}

KDDateProperty::~KDDateProperty() {}

void KDDateProperty::init( bool ) {}

QString KDDateProperty::toString() const
{
  return data().toString();
}

//
//
// KDDateTimeProperty
//
//

/*!\class KDDateTimeProperty KDProperty
  \ingroup KDPropertyView
  \brief A property with a QDateTime value.
*/

class KDDateTimeProperty::Private : public KDAbstractPropertyBase::Private {
    friend class ::KDDateTimeProperty;
public:
    Private( const QDateTime & value, const QString & name )
        : KDAbstractPropertyBase::Private( value, name ) {}
    ~Private() {}
};

KDDateTimeProperty::KDDateTimeProperty( const KDDateTimeProperty & other )
    : KDAbstractProperty<QDateTime>( new Private( *other.d ), true )
{
    init( true );
}

KDDateTimeProperty * KDDateTimeProperty::clone() const
{
    return new KDDateTimeProperty( *this );
}


/*! Constructor. Creates a QDateTime property with name \a name
  and value \a value.
*/
KDDateTimeProperty::KDDateTimeProperty(const QDateTime& value, const QString &name)
  : KDAbstractProperty<QDateTime>( new Private( value, name ), false )
{
    init( false );
}

KDDateTimeProperty::~KDDateTimeProperty() {}

void KDDateTimeProperty::init( bool ) {}

QString KDDateTimeProperty::toString() const
{
  return data().toString();
}


//
//
// KDChoiceProperty
//
//

/*!\class KDChoiceProperty KDProperty
  \ingroup KDPropertyView
  \brief A property with a list of choices.
*/

class KDChoiceProperty::Private : public KDAbstractPropertyBase::Private {
    friend class ::KDChoiceProperty;
public:
    Private( const QStringList & choices_, int value_, const QString & name_ )
        : KDAbstractPropertyBase::Private( value_, name_ ), choices( choices_ ) {}
    ~Private() {}

private:
    QStringList choices;
};

KDChoiceProperty::KDChoiceProperty( const KDChoiceProperty & other )
    : KDAbstractProperty<int>( new Private( *other.d ), true )
{
    init( true );
}

KDChoiceProperty * KDChoiceProperty::clone() const
{
    return new KDChoiceProperty( *this );
}


/*! Constructor. Creates a choice property with name \a name
  and value \a value. The list of choices is initialised from \a choices.
  The actual value of the property is an int describing the index of the
  selected element in the list.
*/
KDChoiceProperty::KDChoiceProperty( const QStringList& choices, int value, const QString& name )
    : KDAbstractProperty<int>( new Private( choices, value, name ), false )
{
    init( false );
}

KDChoiceProperty::~KDChoiceProperty() {}

void KDChoiceProperty::init( bool ) {}

void KDChoiceProperty::setChoices( const QStringList& choices )
{
    d->choices = choices;
}

QStringList KDChoiceProperty::choices() const
{
    return d->choices;
}

QString KDChoiceProperty::toString() const
{
    return QString();
}

QVariant KDChoiceProperty::editorType() const
{
    return QString::fromLatin1("KDChoicePropertyEditor");
}

QVariant KDChoiceProperty::editorHint(int hint) const
{
  if( hint == KDPropertyModel::ChoiceOptionsHint ) {
    return qVariantFromValue(choices());
  } else {
    return QVariant();
  }
}


//
//
// KDMultipleChoiceProperty
//
//

/*!\class KDMultipleChoiceProperty KDProperty
  \ingroup KDPropertyView
  \brief A property with a multiple choices possibility.
*/

class KDMultipleChoiceProperty::Private : public KDAbstractPropertyBase::Private {
    friend class ::KDMultipleChoiceProperty;
public:
    Private( const QStringList & choices_, QList<QVariant> values_, const QString & name_ )
        : KDAbstractPropertyBase::Private( values_, name_ ), choices( choices_ ) {}
    ~Private() {}

private:
    QStringList choices;
};

KDMultipleChoiceProperty::KDMultipleChoiceProperty( const KDMultipleChoiceProperty & other )
    : KDAbstractProperty<QList<QVariant> >( new Private( *other.d ), true )
{
    init( true );
}

KDMultipleChoiceProperty * KDMultipleChoiceProperty::clone() const
{
    return new KDMultipleChoiceProperty( *this );
}


/*! Constructor. Creates a multiple choice property with name \a name
  and value \a value. The list of choices is initialised from \a choices.
  The actual values of the property is an int list describing the index list of the
  selected elements in the list.
*/
KDMultipleChoiceProperty::KDMultipleChoiceProperty( const QStringList& choices, QList<QVariant> values, const QString& name )
    : KDAbstractProperty<QList<QVariant> >( new Private( choices, values, name ), false )
{
    init( false );
}

KDMultipleChoiceProperty::~KDMultipleChoiceProperty() {}

void KDMultipleChoiceProperty::init( bool ) {}

void KDMultipleChoiceProperty::setChoices( const QStringList& choices )
{
    d->choices = choices;
}

QStringList KDMultipleChoiceProperty::choices() const
{
    return d->choices;
}

QString KDMultipleChoiceProperty::toString() const
{
    return QString();
}

QVariant KDMultipleChoiceProperty::editorType() const
{
    return QString::fromLatin1("KDMultipleChoicePropertyEditor");
}

QVariant KDMultipleChoiceProperty::editorHint(int hint) const
{
  if( hint == KDPropertyModel::ChoiceOptionsHint ) {
    return qVariantFromValue(choices());
  } else {
    return QVariant();
  }
}


//
//
// KDColorProperty
//
//


/*!\class KDColorProperty KDProperty
  \ingroup KDPropertyView
  \brief A property with a QColor value.
*/

class KDColorProperty::Private : public KDAbstractPropertyBase::Private {
    friend class ::KDColorProperty;
public:
    Private( const QColor & value, const QString & name )
        : KDAbstractPropertyBase::Private( value, name ) {}
    ~Private() {}
};

KDColorProperty::KDColorProperty( const KDColorProperty & other )
    : KDAbstractProperty<QColor>( new Private( *other.d ), true )
{
    init( true );
}

KDColorProperty * KDColorProperty::clone() const
{
    return new KDColorProperty( *this );
}

/*! Constructor. Creates a QColor property with name \a name
  and value \a value.
*/
KDColorProperty::KDColorProperty( const QColor& value, const QString &name)
    : KDAbstractProperty<QColor>( new Private( value, name ), false )
{
    init( false );
}

KDColorProperty::~KDColorProperty() {}

void KDColorProperty::init( bool ) {}

QString KDColorProperty::toString() const
{
    const QColor c = data();
    return QString::fromLatin1("%1, %2, %3")
    .arg(c.red())
    .arg(c.green())
    .arg(c.blue());
}

QVariant KDColorProperty::editorType() const
{
    return QString::fromLatin1( "KDColorPropertyEditor" );
}

QVariant KDColorProperty::decoration() const
{
    QPixmap pix( 10, 10 );
    pix.fill( data() );
    return pix;
}

//
//
// KDFontProperty
//
//

/*!\class KDFontProperty KDProperty
  \ingroup KDPropertyView
  \brief A property with a QFont value.
*/

class KDFontProperty::Private : public KDAbstractPropertyBase::Private {
    friend class ::KDFontProperty;
public:
    Private( const QFont & value, const QString & name )
        : KDAbstractPropertyBase::Private( value, name ) {}
    ~Private() {}
};

KDFontProperty::KDFontProperty( const KDFontProperty & other )
    : KDAbstractProperty<QFont>( new Private( *other.d ), true )
{
    init( true );
}

KDFontProperty * KDFontProperty::clone() const
{
    return new KDFontProperty( *this );
}

/*! Constructor. Creates a QFont property with name \a name
  and value \a value.
*/
KDFontProperty::KDFontProperty( const QFont& value, const QString &name)
    : KDAbstractProperty<QFont>( new Private( value, name ), false )
{
    init( false );
}

KDFontProperty::~KDFontProperty() {}

void KDFontProperty::init( bool ) {}


QString KDFontProperty::toString() const
{
  QString str;
  // ### !_QObject_::tr
  const QFont f = data();
  if( f.bold() ) {
    if( f.italic() ) str = QObject::tr("Bold Italic ");
    else str = QObject::tr("Bold ");
  } else {
    if( f.italic() ) str = QObject::tr("Italic ");
    else str = QObject::tr("Normal ");
  }
  if( f.strikeOut() ) str += QObject::tr("Strikeout ");
  if( f.underline() ) str += QObject::tr("Underline ");
  return QObject::tr("%1 %2%3 pt.").arg(f.family()).arg(str).arg(f.pointSize());
}

QVariant KDFontProperty::editorType() const
{
    return QString::fromLatin1( "KDFontPropertyEditor" );
}

QVariant KDFontProperty::decoration() const
{
  return QVariant();
}

//
//
// KDSizeProperty
//
//

/*!\class KDSizeProperty KDProperty
  \ingroup KDPropertyView
  \brief A composite property with a QSize value.
*/

class KDSizeProperty::Private : public KDAbstractCompositeProperty::Private {
    friend class ::KDSizeProperty;
public:
    Private( const QString & name )
        : KDAbstractCompositeProperty::Private( name ) {}
    ~Private() {}
};

KDSizeProperty::KDSizeProperty( const KDSizeProperty & other )
    : KDAbstractCompositeProperty( new Private( *other.d ), true )
{
    init( true );
}

KDSizeProperty * KDSizeProperty::clone() const {
    return new KDSizeProperty( *this );
}

/*! Constructor. Creates a QSize property with name \a name
  and value \a value.
*/
KDSizeProperty::KDSizeProperty( const QSize& value, const QString& name )
    : KDAbstractCompositeProperty( new Private( name ), false )
{
    init( false );
    setValue( value );
}

void KDSizeProperty::init( bool copy ) {
    if ( copy )
        return;
    KDPropertyInterface* width = new KDIntProperty( 0,  QObject::tr("Width") );
    KDPropertyInterface* height = new KDIntProperty( 0, QObject::tr("Height") );
    width->setStandalone( false );
    height->setStandalone( false );
    addProperty( width );
    addProperty( height );
}

KDSizeProperty::~KDSizeProperty() {}

QVariant KDSizeProperty::value() const
{
  return qVariantFromValue( QSize( propertyAt(0)->value().toInt(), propertyAt(1)->value().toInt() ) );
}
void KDSizeProperty::setValue( const QVariant& v )
{
  QSize s = qVariantValue<QSize>(v);
  propertyAt(0)->setValue(s.width());
  propertyAt(1)->setValue(s.height());
  update();
}

bool KDSizeProperty::isEditable() const { return false; }

QString KDSizeProperty::toString() const
{
    return QString::fromLatin1("QSize(%1,%2)").arg( propertyAt(0)->value().toInt() ).arg( propertyAt(1)->value().toInt() );
}

//
//
// KDIntWithSliderProperty
//
//

/*!\class KDIntWithSliderProperty KDProperty
  \ingroup KDPropertyView
  \brief A example property with an integer value and a QSlider editor.
*/

class KDIntWithSliderProperty::Private : public KDIntProperty::Private {
    friend class ::KDIntWithSliderProperty;
public:
    Private( int min, int max, int val, const QString & name_ )
      : KDIntProperty::Private( min, max, val, name_) {}
    ~Private();
};


KDIntWithSliderProperty::KDIntWithSliderProperty( const KDIntWithSliderProperty & other )
    : KDIntProperty( new Private( *other.d ), true )
{
    init( true );
}

KDIntWithSliderProperty * KDIntWithSliderProperty::clone() const
{
    return new KDIntWithSliderProperty( *this );
}


/*! Constructor. Creates a integer property with name \a name
  and value \a value.
*/
KDIntWithSliderProperty::KDIntWithSliderProperty(int value, const QString &name)
    : KDIntProperty( new Private(0, 100, value, name), false )
{
    init( false );
}

/*! Constructor. Creates a integer property with name \a name
  and value \a value. \a min and \a max are used for hinting the minimum
  and maximum values to the editor.
*/
KDIntWithSliderProperty::KDIntWithSliderProperty(int min, int max, int value, const QString &name)
    : KDIntProperty( new Private(min, max, value, name), false )
{
    init( false );
}

KDIntWithSliderProperty::~KDIntWithSliderProperty() {}

void KDIntWithSliderProperty::init( bool ) {}

QVariant KDIntWithSliderProperty::editorType() const
{
    return QString::fromLatin1("KDIntSliderPropertyEditor");
}

#undef d
