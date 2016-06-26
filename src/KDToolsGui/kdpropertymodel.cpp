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

#include "kdpropertymodel.h"
#include "kdproperty.h"

#include <QApplication>
#include <QPalette>
#include <QDebug>

/*!\class KDPropertyModel KDPropertyModel
  \ingroup KDPropertyView
  \brief A subclass of QAbstractItemModel for easy construction
  of sets of KDProperties.

  Typically used in conjunction with KDPropertyView.

  Example:

  \code
    KDPropertyCategory* cat = new KDPropertyCategory("A Category");
    cat->addProperty( new KDStringProperty( "foo", "A String" ) );
    cat->addProperty( new KDIntProperty( 42, "An Integer" ) );
    cat->addProperty( new KDSizeProperty( QSize(7,11), "A Composite Property" ) );
    cat->addProperty( new KDColorProperty( QColor( 42,0,211), "A Color Property" ) );

    KDPropertyModel* model = new KDPropertyModel;
    model->addProperty( cat );

    KDPropertyView* pv = new KDPropertyView;
    pv->setModel(model);
    pv->show();
  \endcode
*/

/*! \enum KDPropertyModel::Role
 * An extension to Qt::ItemDataRole for
 * roles specific to KDPropertyModel/KDPropertyView.
 */

/*! \var KDPropertyModel::Role KDPropertyModel::HelpRole
 * This role is used used to communicate extra
 * help text from the model to the view.
 */
/*! \var KDPropertyModel::Role KDPropertyModel::ChangedRole
 * This role is used to communicate if the property was
 * changed from the initial value or not.
 */
/*! \var KDPropertyModel::Role KDPropertyModel::SeparatorRole
 * \todo Do we need this?
 */
/*! \var KDPropertyModel::Role KDPropertyModel::EditorTypeRole
 * This role is used to request a special property editor. If no
 * value is provided for this role, a standard editor based on the
 * QVariant::Type of the EditRole value will be chosen. To request
 * a special editor, set the value of this role to a QString with
 * the classname of the desired editor.
 */
/*! \var KDPropertyModel::Role KDPropertyModel::MinimumHint
 * This role is for providing a hint to editors such as QSliders.
 */
/*! \var KDPropertyModel::Role KDPropertyModel::MinimumHint
 * This role is for providing a hint to editors such as QSliders.
 */
/*! \var KDPropertyModel::Role KDPropertyModel::InputMaskHint
 * This QString role is for providing an inputmask to QLineEdit editors.
 */
/*! \var KDPropertyModel::Role KDPropertyModel::PasswordModeHint
 * This boolean role is for setting QLineEdit editors in password mode.
 */
/*! \var KDPropertyModel::Role KDPropertyModel::ChoiceOptionsHint
 * This role is for providing a QStringList with labels for the
 * options for KDChoiceProperty.
 */

class KDPropertyModel::Private {
    friend class ::KDPropertyModel;
    KDPropertyModel * const q;
public:
    Private( KDPropertyModel * qq )
	: q( qq ),
	  rootproperty( new KDPropertyCategory( QLatin1String( "<ROOT>" ) ) ),
	  flat( false )
    {

    }
    ~Private() {}

private:
    KDPropertyInterface *childAt( KDPropertyInterface* parent, int pos) const;
    KDPropertyInterface *parentOf(KDPropertyInterface *property) const;
    static KDPropertyGroupInterface* toPropertyGroup(KDPropertyInterface *property);

    int dfsIndex( KDPropertyGroupInterface* haystack, KDPropertyInterface* needle, bool* found ) const;
    KDPropertyInterface* dfsCount( KDPropertyGroupInterface* root, int& count ) const;
    int countAll( KDPropertyGroupInterface* root ) const;

private:
    KDPropertyCategory* rootproperty;
    bool flat;
};


/*! Creates a new KDPropertyModel with parent \a parent.
  \see QAbstractItemModel.
 */
KDPropertyModel::KDPropertyModel(QObject* parent)
  : QAbstractItemModel(parent),
    d( new Private( this ) )
{
  d->rootproperty->setModel(this);
}

KDPropertyModel::~KDPropertyModel()
{
  delete d->rootproperty;
}

/*! Adds a property \a prop to this model. The model
  becomes the owner of \a prop
*/
void KDPropertyModel::addProperty( KDPropertyInterface* prop )
{
  d->rootproperty->addProperty(prop);
  reset();
}

/*! Removes the property \a prop from this model. Ownership
  is returned to the caller.
*/
void KDPropertyModel::removeProperty( KDPropertyInterface* prop )
{
  d->rootproperty->removeProperty(prop);
  reset();
}

/*! Returns the number of properties at the toplevel
 */
int KDPropertyModel::propertyCount() const
{
  return d->rootproperty->propertyCount();
}

/*! Returns the toplevel property at index \a idx
 */
KDPropertyInterface* KDPropertyModel::propertyAt(int idx) const
{
  return d->rootproperty->propertyAt(idx);
}

/*! Toggles between flat and tree mode. In flat mode,
  the toplevel categories are hidden. Default is tree
  mode */
void KDPropertyModel::setFlat(bool f)
{
  d->flat = f;
  reset();
}

bool KDPropertyModel::isFlat() const { return d->flat; }

/*!\fn void KDPropertyModel::propertyChanged( KDPropertyInterface* p )
  This signal is emitted when the user has changed the value of
  \a p.
*/

/*! \internal */
QModelIndex KDPropertyModel::indexOf( KDPropertyInterface* property, int column ) const
{
  //qDebug() << "indexOf( " << property << ", " << column <<" )";

    if( isFlat() && property->isStandalone() ) {
    bool found = false;
    int c = d->dfsIndex( d->rootproperty, property, &found );
    if( found ) {
      Q_ASSERT(c>=0);
      //if( c < 0 ) return QModelIndex();
      return createIndex( c, column, property );
    } else {
      return QModelIndex();
    }
  }
  if( property == d->rootproperty)
    return createIndex(0, column, d->rootproperty);

  KDPropertyInterface *parent = d->parentOf(property);
  if( !parent || !parent->isContainer() )
    return QModelIndex();

  int row = static_cast<KDPropertyGroupInterface*>(parent)->indexOf(property);
  Q_ASSERT(row>=0);
  return createIndex(row, column, property);
}

/*! \internal */
KDPropertyInterface *KDPropertyModel::Private::childAt( KDPropertyInterface* parent, int pos) const
{
  if (parent && parent->isContainer() )
    return static_cast<KDPropertyGroupInterface*>(parent)->propertyAt(pos);
  return 0;
}

/*! \internal */
KDPropertyInterface *KDPropertyModel::Private::parentOf(KDPropertyInterface *property) const
{
  return property ? property->parent() : 0;
}

/*! \internal */
KDPropertyGroupInterface* KDPropertyModel::Private::toPropertyGroup(KDPropertyInterface *property)
{
  if (!property || !property->isContainer() )
    return 0;
  return static_cast<KDPropertyGroupInterface*>(property);
}

/*! \internal */
QModelIndex KDPropertyModel::index(int row, int column, const QModelIndex& parent) const
{
  //qDebug() << "index( " << row << ", " << column << ", " <<parent<<" )";
  if ( row < 0 || row >= rowCount( parent ) || column < 0 || column >= columnCount( parent ) )
      return QModelIndex();
  if( isFlat() && (!parent.isValid() || (parent.isValid()
       && static_cast<KDPropertyGroupInterface*>(parent.internalPointer())->isCategory() ) ) ) {
    int c = row;
    KDPropertyInterface* prop = d->dfsCount( d->rootproperty, c );
    if( prop ) return createIndex(row,column,prop);
    else return QModelIndex();
  } else {
    if(!parent.isValid()) {
      Q_ASSERT( d->rootproperty );
      Q_ASSERT( d->childAt(d->rootproperty, row) );
      return createIndex(row, column, d->childAt(d->rootproperty, row));
    }
    Q_ASSERT( privateData(parent) );
    Q_ASSERT( d->childAt(privateData(parent), row) );
    return createIndex(row, column, d->childAt(privateData(parent), row));
  }
}

/*! \internal */
QModelIndex KDPropertyModel::parent(const QModelIndex& index) const
{
  //qDebug() << "parent( " <<index<<" )";
  if(!index.isValid() || privateData(index) == d->rootproperty || d->parentOf(privateData(index)) == d->rootproperty )
    return QModelIndex();

  Q_ASSERT(privateData(index));

  /* Only sub-properties can have parents in flat mode */
  if( isFlat() && privateData(index)->isStandalone() ) return QModelIndex();

  return indexOf(d->parentOf(privateData(index)));
}

/*! \internal */
int KDPropertyModel::rowCount(const QModelIndex& parent) const
{
  if (!parent.isValid()) {
    if( isFlat() ) return d->countAll(d->rootproperty);
    else return d->rootproperty->propertyCount();
  }

  if (KDPropertyInterface *p = privateData(parent)) {
    return (p->isContainer())
      ? static_cast<KDPropertyGroupInterface*>(p)->propertyCount()
      : 0;
  }

  return (d->rootproperty->isContainer() )
    ? static_cast<KDPropertyGroupInterface*>(d->rootproperty)->propertyCount()
    : 0;
}

/*! \internal */
int KDPropertyModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return 2;
}

bool KDPropertyModel::hasChildren( const QModelIndex & parent ) const
{
    return rowCount( parent ) > 0; // ### isn't that the default anyway?
}

/*! \internal */
bool KDPropertyModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if(KDPropertyInterface *property = privateData(index)) {
    if (role == Qt::EditRole /*|| role == ChangedRole*/ ) {
      if( value != property->value() ) {
        property->setValue(value);
        property->setChanged(true);

        KDPropertyInterface* stdalone = property;
        while(stdalone != 0 && !stdalone->isStandalone())
          stdalone = stdalone->parent();
        if (stdalone != 0 && stdalone != property /*&& nonfake->dirty()*/) {
          /*nonfake->setDirty(false);*/ // TODO
          emit propertyChanged(stdalone);
        }
      }
    } else if( role == Qt::CheckStateRole && property->isCategory() ) {
      KDPropertyCategory* cat = static_cast<KDPropertyCategory*>(property);
      cat->setChecked( qVariantValue<int>(value)==Qt::Checked );
      update(cat);
    }
    return true;
  }
  return false;
}

/*! \internal */
QVariant KDPropertyModel::data(const QModelIndex& index, int role) const
{
  if (!privateData(index))
    return QVariant();

  KDPropertyInterface *o = privateData(index);
  Q_ASSERT(o);
  switch (index.column()) {
  case 0:
    switch (role) {
    case Qt::EditRole:
    case Qt::DisplayRole:
      return o->name().isEmpty()
	? QLatin1String("<noname>")
	: o->name();
    case Qt::DecorationRole:
      if( o->isCategory() ) return o->decoration();
      break;
    case Qt::BackgroundColorRole:
      return o->isCategory()
	?QApplication::palette().color(QPalette::AlternateBase)
	:QApplication::palette().color(QPalette::Base);
    case Qt::CheckStateRole:
      return (o->isCategory()&&static_cast<KDPropertyCategory*>(o)->isCheckable())
        ?qVariantFromValue(static_cast<int>(static_cast<KDPropertyCategory*>(o)->isChecked()))
	:QVariant();
	break;
    case ChangedRole:
      return o->isChanged();
    case HelpRole:
      return o->helpText();
    default:
      break;
    }
    break;
  case 1: {
    switch (role) {
    case Qt::EditRole:
      return o->value();
    case Qt::DisplayRole:
      return o->toString();
    case Qt::DecorationRole:
      if( !o->isCategory() ) return o->decoration();
      break;
    case Qt::BackgroundColorRole:
      return o->isCategory()
	?QApplication::palette().color(QPalette::AlternateBase)
	:QApplication::palette().color(QPalette::Base);
    case ChangedRole:
      return o->isChanged();
    case EditorTypeRole:
      return o->editorType();
    case HelpRole:
      return o->helpText();
    default:
      QVariant v = o->editorHint(role);
      if( v.isValid() ) return v;
      break;
    }
    break;
  }
  default:
    break;
  }
  return QVariant();
}

/*! \internal */
bool KDPropertyModel::isEditable(const QModelIndex& index) const
{
  return index.column() == 1 && privateData(index) && privateData(index)->isEditable();
}

/*! \internal */
KDPropertyInterface* KDPropertyModel::privateData(const QModelIndex &index) const
{
  return static_cast<KDPropertyInterface*>(index.internalPointer());
}

/*! \internal */
bool KDPropertyModel::isEnabled(const QModelIndex& index) const
{
  return privateData(index) && privateData(index)->isEnabled();
}

/*! \internal */
QModelIndex KDPropertyModel::buddy(const QModelIndex& index) const
{
  if (index.column() == 0 && privateData(index) && !privateData(index)->isCategory() )
    return createIndex(index.row(), 1, index.internalPointer());
  return index;
}

/*! \internal */
QVariant KDPropertyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal) {
    if (role != Qt::DisplayRole)
      return QVariant();
    switch(section) {
    case 0: return tr("Property");
    case 1: return tr("Value");
    default: return QString();
    }
  }
  return QAbstractItemModel::headerData(section, orientation, role);
}

/*! \internal */
Qt::ItemFlags KDPropertyModel::flags(const QModelIndex &index) const
{
  Qt::ItemFlags f = QAbstractItemModel::flags(index);
  if(isEnabled(index)) {
    f |= Qt::ItemIsEnabled;
    if(isEditable(index))
      f |= Qt::ItemIsEditable;
  } else {
    f &= !Qt::ItemIsEnabled;
  }
  if(privateData(index) && privateData(index)->isCategory())
    f |= Qt::ItemIsUserCheckable|Qt::ItemIsEnabled;
  return f;
}

int KDPropertyModel::Private::dfsIndex( KDPropertyGroupInterface* haystack,
					KDPropertyInterface* needle, bool* found ) const
{
  //qDebug() << "dfsIndex( " << haystack->name() << ", " << needle->name() << " )";
  if( haystack == needle ) {
    *found = true;
    return 0;
  }

  int c = 0;
  for( int i = 0; i < haystack->propertyCount(); ++i ) {
    KDPropertyInterface* iprop = haystack->propertyAt(i);
    if( iprop->isCategory() ) {
      c += dfsIndex( static_cast<KDPropertyGroupInterface*>(iprop), needle, found );
      if( *found ) return c;
    } else {
      ++c;
    }
    if( needle==iprop ) {
      *found = true;
      return c;
    }
  }
  //qDebug() << "dfsIndex did not find " << needle <<" in " << haystack;
  *found = false;
  return c;
}

KDPropertyInterface* KDPropertyModel::Private::dfsCount( KDPropertyGroupInterface* root, int& count ) const
{
  //qDebug() << "dfsCount( " << root << ", " << count << " )";
  for( int i = 0; i < root->propertyCount(); ++i ) {
    KDPropertyInterface* prop = root->propertyAt(i);
    if( prop->isCategory() ) {
      prop = dfsCount( static_cast<KDPropertyGroupInterface*>(prop),count);
      if( count == 0 && prop ) return prop;
    } else {
      if( count == 0 ) return prop;
      else --count;
    }
  }
  return 0;
}

int KDPropertyModel::Private::countAll( KDPropertyGroupInterface* root ) const
{
  int res = 0;
  for( int i = 0; i < root->propertyCount(); ++i ) {
    KDPropertyInterface* prop = root->propertyAt(i);
    if( prop->isCategory() ) {
      res += countAll(static_cast<KDPropertyGroupInterface*>(prop));
    } else {
      ++res;
    }
  }
  return res;
}

/*! \internal */
void KDPropertyModel::update(KDPropertyInterface* prop)
{
  emit dataChanged(indexOf(prop,0),indexOf(prop,1));
  emit propertyChanged(prop);
#if 0
  KDPropertyInterface* p = prop;
  while( p ) {
    emit dataChanged(indexOf(p,0),indexOf(p,1));
    p = p->parent();
  }

  if( prop->isContainer() ) {
    KDPropertyGroupInterface* g = static_cast<KDPropertyGroupInterface*>(prop);
    for( int i = 0; i < g->propertyCount(); ++i ) {
      KDPropertyInterface* c = g->propertyAt(i);
      emit dataChanged(indexOf(c,0),indexOf(c,1));
    }
  }
#endif
}

/*! \internal */
void KDPropertyModel::update()
{
  update(d->rootproperty);
}

#include "moc_kdpropertymodel.cpp"
