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

#include "kdupdaterupdateoperation.h"

#include <QDebug>
#include <QMap>
#include <QVariant>

#include <QDomDocument>
#include <QDomElement>
#include <QStringList>

/*!
   \ingroup kdupdater
   \class KDUpdater::UpdateOperation kdupdaterupdateoperation.h KDUpdaterUpdateOperation
   \brief Abstract base class for update operations.

   The \ref KDUpdater::UpdateOperation is an abstract class that specifies an interface for
   update operations. Concrete implementations of this class must perform a single update
   operation like copy, move, delete etc.

   \note Two separate threads cannot be using a single instance of KDUpdater::UpdateOperation
   at the same time.
*/

using namespace KDUpdater;

class UpdateOperation::Private
{
public:
    Private( UpdateOperation* qq ) :
        q( qq ),
        error( 0 ),
        target( 0 )
    {}

    UpdateOperation* q;
    QString name;
    QStringList args;
    int error;
    Target * target;
    QString errorMessage;
    QVariantMap values;
};


/*!
   Constructor
*/
UpdateOperation::UpdateOperation()
    : d ( new Private( this ) )
{
}

/*!
   Destructor
*/
UpdateOperation::~UpdateOperation()
{
}

/*!
   Returns the update operation name.

   \sa setName()
*/
QString UpdateOperation::name() const
{
    return d->name;
}

/*!
   Returns a command line string that describes the update operation. The returned
   string would be of the form

   <name> <arg1> <arg2> <arg3> ....
*/
QString UpdateOperation::operationCommand() const
{
    const QString argsStr = d->args.join(QLatin1String( " " ));
    return QString::fromLatin1( "%1 %2" ).arg(d->name, argsStr);
}

/*!
  This method returns the last error message as a string
*/
QString UpdateOperation::lastError() const
{
    return d->errorMessage;
}

/*!
 Returns true if there exists a setting called \a name. Otherwise returns false.
*/
bool UpdateOperation::hasValue( const QString& name ) const
{
    return d->values.contains( name );
}

/*!
 Clears the value of setting \a name and removes it.
 \post hasValue( \a name ) returns false.
*/
void UpdateOperation::clearValue( const QString& name )
{
    d->values.remove( name );
}

/*!
 Returns the value of setting \a name. If the setting does not exists,
 this returns an empty QVariant.
*/
QVariant UpdateOperation::value( const QString& name ) const
{
    return d->values.value( name );
}

/*!
 \internal
 Returns all values set.
*/
QVariantMap UpdateOperation::values() const
{
    return d->values;
}

/*!
 \internal
 Sets all set values to \a values
 All values set previously are overwritten.
*/
void UpdateOperation::setValues( const QVariantMap& values )
{
    d->values = values;
}

/*!
  Sets the value of setting \a name to \a value.
*/
void UpdateOperation::setValue( const QString& name, const QVariant& value )
{
    d->values[ name ] = value;
}

/*!
   Sets a update operation name. Subclasses will have to provide a unique
   name to describe this operation.
*/
void UpdateOperation::setName(const QString& name)
{
    d->name = name;
}

/*!
   Through this function, arguments to the update operation can be specified
   to the update operation.
*/
void UpdateOperation::setArguments(const QStringList& args)
{
    d->args = args;
}

/*!
   Sets the Target for this operation.
   This may be used by some operations
*/
void UpdateOperation::setTarget( Target * target )
{
    d->target = target;
}

/*!
   Returns the last set function arguments.
*/
QStringList UpdateOperation::arguments() const
{
    return d->args;
}

/*!
   Returns error details in case performOperation() failed.
*/
QString KDUpdater::UpdateOperation::errorString() const
{
    return d->errorMessage;
}

/*!
   Can be used by subclasses to report more detailed error codes (optional).
   To check if an operation was successful, use the return value of performOperation().
*/
int KDUpdater::UpdateOperation::error() const
{
    return d->error;
}

/*!
 * Used by subclasses to set the error string.
 */
void KDUpdater::UpdateOperation::setErrorString( const QString& errorMessage )
{
    d->errorMessage = errorMessage;
}

/*!
   Used by subclasses to set the error code.
*/
void KDUpdater::UpdateOperation::setError( int error )
{
    d->error = error;
}

/*!
   Used by subclasses to set the error code and the error string.
*/
void KDUpdater::UpdateOperation::setError( int error, const QString& errorMessage )
{
    d->error = error;
    d->errorMessage = errorMessage;
}

/*!
   Returns a pointer to the current Target
*/
Target * UpdateOperation::target() const
{
    return d->target;
}

/*!
   \fn virtual void KDUpdater::UpdateOperation::backup() = 0;

   Subclasses must implement this function to backup any data before performing the action.
*/

/*!
   \fn virtual bool KDUpdater::UpdateOperation::performOperation() = 0;

   Subclasses must implement this function to perform the update operation
*/

/*!
   \fn virtual bool KDUpdater::UpdateOperation::undoOperation() = 0;

   Subclasses must implement this function to perform the reverse of the operation.
*/

/*!
   \fn virtual bool KDUpdater::UpdateOperation::testOperation() = 0;

   Subclasses must implement this function to perform the test operation.
*/

/*!
   \fn virtual KDUpdater::UpdateOperation* KDUpdater::UpdateOperation::clone() const = 0;

   Subclasses must implement this function to clone the current operation.
*/

/*!
  Saves this UpdateOperation in XML. You can override this method to store your own extra-data.
  The default implementation is taking care of arguments and values set via setValue.
*/
QDomDocument KDUpdater::UpdateOperation::toXml() const
{
    QDomDocument doc;
    QDomElement root = doc.createElement( QLatin1String("operation") );
    doc.appendChild( root );
    QDomElement args = doc.createElement( QLatin1String("arguments") );
    Q_FOREACH( const QString &s, arguments() ) {
        QDomElement arg = doc.createElement( QLatin1String("argument") );
        arg.appendChild( doc.createTextNode(s) );
        args.appendChild( arg );
    }
    root.appendChild( args );
    if( d->values.isEmpty() )
        return doc;

    // append all values set with setValue
    QDomElement values = doc.createElement( QLatin1String( "values" ) );
    for( QVariantMap::const_iterator it = d->values.begin(); it != d->values.end(); ++it )
    {
        QDomElement value = doc.createElement( QLatin1String( "value" ) );
        const QVariant& variant = it.value();
        value.setAttribute( QLatin1String( "name" ), it.key() );
        value.setAttribute( QLatin1String( "type" ), QLatin1String( QVariant::typeToName( variant.type() ) ) );

        if( variant.type() != QVariant::List && variant.type() != QVariant::StringList && qVariantCanConvert< QString >( variant ) )
        {
            // it can convert to string? great!
            value.appendChild( doc.createTextNode( variant.toString() ) );
        }
        else
        {
            // no? then we have to go the hard way...
            QByteArray data;
            QDataStream stream( &data, QIODevice::WriteOnly );
            stream << variant;
            value.appendChild( doc.createTextNode( QLatin1String( data.toBase64().data() ) ) );
        }
        values.appendChild( value );
    }
    root.appendChild( values );
    return doc;
}

/*!
  Restores UpdateOperation's arguments and values from the XML document \a doc.
  Returns true on success, otherwise false.
*/
bool KDUpdater::UpdateOperation::fromXml( const QDomDocument &doc )
{
    QStringList args;
    const QDomElement root = doc.documentElement();
    const QDomElement argsElem = root.firstChildElement( QLatin1String("arguments") );
    Q_ASSERT( ! argsElem.isNull() );
    for( QDomNode n = argsElem.firstChild(); ! n.isNull(); n = n.nextSibling() ) {
        const QDomElement e = n.toElement();
        if( !e.isNull() && e.tagName() == QLatin1String("argument") )
            args << e.text();
    }
    setArguments(args);

    d->values.clear();
    const QDomElement values = root.firstChildElement( QLatin1String( "values" ) );
    for( QDomNode n = values.firstChild(); !n.isNull(); n = n.nextSibling() )
    {
        const QDomElement v = n.toElement();
        if( v.isNull() || v.tagName() != QLatin1String( "value" ) )
            continue;

        const QString name = v.attribute( QLatin1String( "name" ) );
        const QString type = v.attribute( QLatin1String( "type" ) );
        const QString value = v.text();

        const QVariant::Type t = QVariant::nameToType( type.toLatin1().data() );
        QVariant var = qVariantFromValue( value );
        if( t == QVariant::List || t == QVariant::StringList || !var.convert( t ) )
        {
            QDataStream stream( QByteArray::fromBase64( value.toLatin1() ) );
            stream >> var;
        }

        d->values[ name ] = var;
    }

    return true;
}

/*!
  Restores UpdateOperation's arguments and values from the XML document at path \a xml.
  Returns true on success, otherwise false.
  \overload
*/
bool KDUpdater::UpdateOperation::fromXml( const QString &xml )
{
    QDomDocument doc;
    QString errorMsg;
    int errorLine;
    int errorColumn;
    if ( ! doc.setContent( xml, &errorMsg, &errorLine, &errorColumn ) ) {
        qWarning() << "Error parsing xml error=" << errorMsg << "line=" << errorLine << "column=" << errorColumn;
        return false;
    }
    return fromXml(doc);
}
