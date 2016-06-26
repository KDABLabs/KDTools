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

#include "kdvariantconverter.h"

#if QT_VERSION >= 0x040200 || defined( DOXYGEN_RUN )

#include <QDateTime>
#include <QDebug>

using namespace kdtools;

/*!
  \class KDVariantConverter
  \ingroup core
  \brief A helper for making user types in QVariant convertable.
  \since_c 2.2

  KDVariantConverter provides a away to make user types stored in QVariant not only usable with
  QVariant::toString(), QVariant::toInt()... but also comparable.

  The most common use case for QVariant is to hand them out via #QAbstractItemModel::data.
  Once fetched from the model, most views then make them visible to the user by calling QVariant::toString
  on them. Unfortunately, QVariant::toString returns an empty string for user types. This is where
  KDVariantConverter can help. The following line is enough to get \c MyType::toString() called whenever
  the QVariant containing \c MyType should be displayed:

  \code
  KDVariantConverter::registerConversion< MyType >();
  \endcode

  \section Comparison

  KDVariantConverter offers an operator< for QVariant types. This one can be used to compare and sort QVariants, given
  that your user type contained in it is registered in KDVariantConverter and there's an operator< for the user type.

  If you want to sort your QVariants in a QSortFilterProxyModel you need to reimplement lessThan, since the default
  implemention is doing a string-based comparison for all non-numeric types. Since KDVariantConverter's operator< follows
  the same comparison rules for Qt's default types as QSortFilterProxyModel, this is easy:

  \code
  CustomSortFilterProxyModel::lessThan( const QModelIndex& lhs, const QModelIndex& rhs ) const
  {
    return lhs.data( sortRole() ) < rhs.data( sortRole() );
  }
  \endcode

  \sa registerConversion

 */

/*!
  \fn int KDVariantConverter::registerConversion< TYPE, VARIANT_TYPE >()

  Register \a converter to be used for conversion from a QVariant
  with type TYPE to VARIANT_TYPE.

  For this to work you need to provide a method TYPE::toXXX(), where XXX is
  the proper type name. See the QVariant documentation for the list of types
  available.

  VARIANT_TYPE defaults to QVariant::String, which results to a conversion to
  QString via TYPE::toString().

  If TYPE has a matching operator== or operator< it gets even used for
  QVariant comparison via QVariant's operator== or operator< privoded by
  KDVariantConverter.

  \note TYPE must have been declaced as a QMetaType.
  \note The conversion is only guaranteed to work with QVariant::toXXX(),
         qVariantValue< TYPE >() works only for non-numeric types like QString.

  The following example code registers \c MyType to be convertable to double. The
  conversion is done via QVariant::toDouble which accesses \c MyType::toDouble.
  \code
  class MyType
  {
  public:
      double toDouble( bool* ok = 0 ) const
      {
          if( ok )
              *ok = true;
          return 3.14;
      }
  };

  Q_DECLARE_METATYPE( MyType )

  int main()
  {
      KDVariantConverter::registerConversion< MyType, QVariant::Double >();
      MyType mt;
      QVariant var = qVariantFromValue< MyType >( mt );
      double d = var.toDouble(); // returns 3.14
  }
  \endcode
 */

std::map< qvariant_conversion_registrar::converter, converter_function >* qvariant_conversion_registrar::converters = 0;
std::map< int, comparator_function >* qvariant_conversion_registrar::comparators_equals = 0;
std::map< int, comparator_function >* qvariant_conversion_registrar::comparators_less_than = 0;

const QVariant::Handler* oldHandler = 0;

/*! QVariant's original compare function pointer. */
static QVariant::f_compare oldCompare = 0;
/*! QVariant's original convert function pointer. */
static QVariant::f_convert oldConvert = 0;
#if !defined(QT_NO_DEBUG_STREAM) && !defined(Q_BROKEN_DEBUG_STREAM)
/*! QVariant's original debugStream function pointer. */
static QVariant::f_debugStream oldStreamDebug = 0;
#endif
/*! QVariant's original construct function pointer. */
static QVariant::f_construct oldConstruct = 0;
/*! QVariant's original clear function pointer. */
static QVariant::f_clear oldClear = 0;

/*!
 \struct comparator_function
 \internal
 \brief comparator_function functor.

 This class keeps a pointer to a QVariant::f_compare function
 and passes it's parameters to it.
 */

/*!
  \struct converter_function
  \internal
  \brief converter_function functor.

  This class keeps a pointer to a QVariant::f_convert function
  and passes it's parameters to it.
 */

/*!
 \internal
 \class qvariant_conversion_register
 Helper class providing registerIt, which needs access to protocted
 QVariant data. This can't be instantiated, since QVariant should not
 be derived.
 */

/*!
 Creates a null comparator_function.
 */
comparator_function::comparator_function()
    : function( 0 )
{
}

/*!
 Returns true, when a valid function has been set. False otherwise.
*/
comparator_function::operator bool() const
{
    return function != 0;
}

/*!
 Calls the associatied function with \a a and \a b.
 */
bool comparator_function::operator()( const QVariant::Private* a, const QVariant::Private* b ) const
{
    if( function != 0 )
        return function( a, b );
    return false;
}

/*!
 Creates a comparator_function using \a function.
 */
comparator_function::comparator_function( QVariant::f_compare fun )
    : function( fun )
{
}

/*!
 Creates a null converter_function.
 */
converter_function::converter_function()
    : function( 0 )
{
}

/*!
 Returns true, when a valid function has been set. False otherwise.
 */
converter_function::operator bool() const
{
    return function != 0;
}

/*!
 Calls the associated function with \a d, \a t, \a result and \a ok.
 */
#if QT_VERSION >= 0x050000
bool converter_function::operator()( const QVariant::Private* d, int t, void* result, bool* ok ) const
#else
bool converter_function::operator()( const QVariant::Private* d, QVariant::Type t, void* result, bool* ok ) const
#endif
{
    if( function != 0 )
        return function( d, t, result, ok );
    if( ok )
        *ok = false;
    return false;
}

/*!
 Creates a new converter_function using \a function.
 */
converter_function::converter_function( QVariant::f_convert fun )
    : function( fun )
{
}

#if !defined(QT_NO_DEBUG_STREAM) && !defined(Q_BROKEN_DEBUG_STREAM)

/*!
 \internal
 Wrapper function for QVariant's old streamDebug function.
 */
static void myStreamDebug( QDebug dbg, const QVariant& v )
{
    if( v.type() < QVariant::UserType )
    {
        oldStreamDebug( dbg, v );
        return;
    }

    dbg.nospace() << v.toString();
}
#endif

/*!
 \internal
 Creates a converter with \a id and \a targetType.
*/
#if QT_VERSION >= 0x050000
qvariant_conversion_registrar::converter::converter( int id_, int tt )
#else
qvariant_conversion_registrar::converter::converter( int id_, QVariant::Type tt )
#endif
    : id( id_ ),
      targetType( tt )
{
}

/*!
 \internal
 Wrapper function for QVariant's old compare function.
 */
static bool myCompare( const QVariant::Private* a, const QVariant::Private* b )
{
    if( a->type < QVariant::UserType || b->type < QVariant::UserType || a->type != b->type )
        return oldCompare( a, b );

    if( qvariant_conversion_registrar::comparators_equals == 0 )
        return oldCompare( a, b );

    comparator_function& compare = (*qvariant_conversion_registrar::comparators_equals)[ a->type ];
    if( !compare )
        return oldCompare( a, b );

    return compare( a, b );
}

/*!
 \internal
 Wrapper function for QVariant's old convert function.
 */
#if QT_VERSION >= 0x050000
static bool myConvert( const QVariant::Private* d, int t, void* result, bool* ok )
#else
static bool myConvert( const QVariant::Private* d, QVariant::Type t, void* result, bool* ok )
#endif
{
    if( d->type < QVariant::UserType )
        return oldConvert( d, t, result, ok );

    if( qvariant_conversion_registrar::converters == 0 )
        return oldConvert( d, t, result, ok );

    converter_function& convert = (*qvariant_conversion_registrar::converters)[ qvariant_conversion_registrar::converter( d->type, t ) ];
    if( !convert )
        return oldConvert( d, t, result, ok );

    return convert( d, t, result, ok );
}

/*!
 \internal
 Wrapper function for QVariant's old construct function.
 */
static void myConstruct( QVariant::Private* x, const void* copy )
{
    oldConstruct( x, copy );
}

/*!
 \internal
 Wrapper function for QVariant's old clear function.
 */
static void myClear( QVariant::Private* d )
{
    oldClear( d );
}

#if QT_VERSION >= 0x050000
QT_BEGIN_NAMESPACE
extern const QVariant::Handler *qcoreVariantHandler();
QT_END_NAMESPACE
#endif

/*
 \internal
 Registers the wrapper functions inside of QVariant.
 */
void qvariant_conversion_registrar::registerIt()
{
#if QT_VERSION >= 0x050000
    const QVariant::Handler *handler = qcoreVariantHandler();
#endif

    // this is why we need to derive QVariant...
    static const QVariant::Handler kdab_qt_kernel_variant_handler = {
        myConstruct,
        myClear,
        handler->isNull,
#ifndef QT_NO_DATASTREAM
        handler->load,
        handler->save,
#endif
        myCompare,
        myConvert,
        handler->canConvert,
#if !defined(QT_NO_DEBUG_STREAM) && !defined(Q_BROKEN_DEBUG_STREAM)
        myStreamDebug
#else
        0
#endif
    };

    if( handler != &kdab_qt_kernel_variant_handler )
    {
        oldHandler = handler;
        oldCompare = handler->compare;
        oldConvert = handler->convert;
        oldConstruct = handler->construct;
        oldClear = handler->clear;
#if !defined(QT_NO_DEBUG_STREAM) && !defined(Q_BROKEN_DEBUG_STREAM)
        oldStreamDebug = handler->debugStream;
#endif
        handler = &kdab_qt_kernel_variant_handler;
    }

    if( qvariant_conversion_registrar::converters == 0 )
        qvariant_conversion_registrar::converters = new std::map< qvariant_conversion_registrar::converter, converter_function >;

    if( qvariant_conversion_registrar::comparators_equals == 0 )
        qvariant_conversion_registrar::comparators_equals = new std::map< int, comparator_function >;

    if( qvariant_conversion_registrar::comparators_less_than == 0 )
        qvariant_conversion_registrar::comparators_less_than = new std::map< int, comparator_function >;

#if QT_VERSION >= 0x050000
    (*const_cast<QVariant::Handler*>(qcoreVariantHandler())) = kdab_qt_kernel_variant_handler;
#endif
}

/*!
 \internal
 To be able to put it into a map.
 */
bool kdtools::operator<( const qvariant_conversion_registrar::converter& lhs, const qvariant_conversion_registrar::converter& rhs )
{
    if( lhs.id != rhs.id )
        return lhs.id < rhs.id;
    else
        return lhs.targetType < rhs.targetType;
}

namespace {
    // QVariant::data_ptr() is undocumented API in Qt >= 4.3, but 'd'
    // is protected in all Qt 4 versions, so we can work around that:
    class QVariantDataPtrAccess : public QVariant {
    public:
        typedef Private DataPtr;
        DataPtr & data_ptr() { return d; }
    };
}

/*!
 Returns true if \a lhs is less than \a rhs.
 The comparison is done depending on the type of \a lhs and \a rhs. For user types, it tries
 to use an operator< for the type. If this fails, it falls back to comparing pointers.

 Comparison of different types might be undefined.
 \relates KDVariantConverter
 */
bool operator<( const QVariant& lhs, const QVariant& rhs )
{
    if( lhs.type() < QVariant::UserType || rhs.type() < QVariant::UserType || lhs.type() != rhs.type() )
    {
        switch( lhs.type() )
        {
        case QVariant::Int:
            return lhs.toInt() < rhs.toInt();
        case QVariant::UInt:
            return lhs.toUInt() < rhs.toUInt();
        case QVariant::LongLong:
            return lhs.toLongLong() < rhs.toLongLong();
        case QVariant::ULongLong:
            return lhs.toULongLong() < rhs.toULongLong();
        case QVariant::Double:
            return lhs.toDouble() < rhs.toDouble();
        case QVariant::Char:
            return lhs.toChar() < rhs.toChar();
        case QVariant::Date:
            return lhs.toDate() < rhs.toDate();
        case QVariant::Time:
            return lhs.toTime() < rhs.toTime();
        case QVariant::DateTime:
            return lhs.toDateTime() < rhs.toDateTime();
        default:
            return lhs.toString() < rhs.toString();
        }
    }

    if( qvariant_conversion_registrar::comparators_less_than == 0 )
        return lhs.toString() < rhs.toString();

    comparator_function& compare = (*qvariant_conversion_registrar::comparators_less_than)[ lhs.userType() ];
    if( !compare )
        return lhs.toString() < rhs.toString();

    return compare( &static_cast< QVariantDataPtrAccess& >( const_cast< QVariant& >( lhs ) ).data_ptr(),
                    &static_cast< QVariantDataPtrAccess& >( const_cast< QVariant& >( rhs ) ).data_ptr() );
}

#ifdef KDTOOLSCORE_UNITTESTS

#include <KDUnitTest/Test>

#include <QBitArray>
#include <QDateTime>
#ifndef QT_NO_GEOM_VARIANT
#include <QLineF>
#include <QRectF>
#endif
#include <QLocale>
#ifndef QT_NO_REGEXP
#include <QRegExp>
#endif
#include <QStringList>
#include <QUrl>

//
// (marc)
// Qt 4.1 and 4.2 crash in optimized builds on linux-32 when QMetaType
// is instantiated with a class from the anonymous namespace. So we
// make our own anonymous namespace here, to work around that.
//
namespace _qvariant_converter_uniq_1389392 {

class MyClass
{
public:
    QBitArray toBitArray() const { QBitArray b( 8 ); for( int i = 0; i < b.size(); i += 2 ) b.toggleBit( i ); return b; }
    bool toBool() const { return true; }
    QByteArray toByteArray() const { return QByteArray( "MyClass::toByteArray" ); }
    QChar toChar() const { return QChar::fromLatin1( '@' ); }
    QDate toDate() const { return toDateTime().date(); }
    QDateTime toDateTime() const { QDateTime result; result.setTime_t( 1234567890 ); return result; }
    double toDouble( bool* ok = 0 ) const { if( ok ) *ok = true; return 3.14; }
    int toInt( bool* ok = 0 ) const { if( ok ) *ok = true; return -5; }
#ifndef QT_NO_GEOM_VARIANT
    QLine toLine() const { return toLineF().toLine(); }
    QLineF toLineF() const { return QLineF( toRect().topLeft(), toRect().bottomRight() ); }
#endif
    QList< QVariant > toList() const { QList< QVariant > list; list += ( toMap().values() ); return list; }
    QLocale toLocale() const { return QLocale( QLocale::Swedish, QLocale::Sweden ); }
    qlonglong toLongLong( bool* ok = 0 ) const { if( ok ) *ok = true; return -38292; }
    QMap< QString, QVariant > toMap() const { QMap< QString, QVariant > map; map[ toString() ] = qVariantFromValue( 4 ); return map; }
#ifndef QT_NO_GEOM_VARIANT
    QPoint toPoint() const { return toPointF().toPoint(); }
    QPointF toPointF() const { return toRectF().topLeft(); }
    QRect toRect() const { return toRectF().toRect(); }
    QRectF toRectF() const { return QRectF( toDouble(), -toDouble(), 2 * toDouble(), -4 * toDouble() ); }
#endif
#ifndef QT_NO_REGEXP
    QRegExp toRegExp() const { return QRegExp( QString::fromLatin1( "a.*b" ) ); }
#endif
#ifndef QT_NO_GEOM_VARIANT
    QSize toSize() const { return toSizeF().toSize(); }
    QSizeF toSizeF() const { return toRectF().size(); }
#endif
    QString toString() const { return QString::fromLatin1( "MyClass::toString()" ); }
    QStringList toStringList() const { return QStringList() << toString(); }
    QTime toTime() const { return toDateTime().time(); }
    uint toUInt( bool* ok = 0 ) const { if( ok ) *ok = true; return 934930; }
    qulonglong toULongLong( bool* ok = 0 ) const { if( ok ) *ok = true; return 123456; }
    QUrl toUrl() const { return QUrl( QString::fromLatin1( "http://www.kdab.net/" ) ); }
};

class MyClass2 : public MyClass
{
public:
    MyClass2( int dat = 0 )
        : data( dat )
    {
    }

    const int data;
};

bool operator==( const MyClass2& lhs, const MyClass2& rhs )
{
    return lhs.data == rhs.data;
}

bool operator<( const MyClass2& lhs, const MyClass2& rhs )
{
    return lhs.data < rhs.data;
}

} // namespace _qvariant_converter_uniq_1389392
using namespace _qvariant_converter_uniq_1389392;

QT_BEGIN_NAMESPACE
static inline std::ostream& operator<<( std::ostream& stream, const QBitArray& ba )
{
    stream << "QBitArray(";
    for( int i = 0; i < ba.size(); ++i )
    {
        stream << ( ba.testBit( i ) ? " 1" : " 0" );
        if( i != ba.size() - 1 )
            stream << ",";
    }
    stream << " )";
    return stream;
}

static inline std::ostream& operator<<( std::ostream& stream, const QByteArray& ba )
{
    stream << "QByteArray( " << ba.data() << " )";
    return stream;
}

static inline std::ostream& operator<<( std::ostream& stream, const QDate& d )
{
    stream << "QDate( " << d.toString().toLocal8Bit().data() << " )";
    return stream;
}

static inline std::ostream& operator<<( std::ostream& stream, const QDateTime& d )
{
    stream << "QDate( " << d.toString().toLocal8Bit().data() << " )";
    return stream;
}

static inline std::ostream& operator<<( std::ostream& stream, const QPoint& point )
{
    stream << "QPoint( " << point.x() << ", " << point.y() << " )";
    return stream;
}

static inline std::ostream& operator<<( std::ostream& stream, const QPointF& point )
{
    stream << "QPointF( " << point.x() << ", " << point.y() << " )";
    return stream;
}

static inline std::ostream& operator<<( std::ostream& stream, const QLine& line )
{
    stream << "QLine( " << line.p1() << ", " << line.p2() << " )";
    return stream;
}

static inline std::ostream& operator<<( std::ostream& stream, const QLineF& line )
{
    stream << "QLineF( " << line.p1() << ", " << line.p2() << " )";
    return stream;
}

static inline std::ostream& operator<<( std::ostream& stream, const QList< QVariant >& list )
{
    stream << "QList< QVariant >(";
    for( QList< QVariant >::const_iterator it = list.begin(); it != list.end(); ++it )
    {
        stream << " " << it->toString().toLocal8Bit().data();
        if( it + 1 != list.end() )
            stream << ",";
    }
    stream << " )";
    return stream;
}

#if 0
static inline std::ostream& operator<<( std::ostream& stream, const QLocale& locale )
{
    stream << "QLocale( " << locale.name() << " )";
    return stream;
}
#endif

static inline std::ostream& operator<<( std::ostream& stream, const QMap< QString, QVariant >& map )
{
    stream << "QMap( ";
    for( QMap< QString, QVariant >::const_iterator it = map.begin(); it != map.end(); ++it )
    {
        stream << "( " << it.key().toLocal8Bit().data() << ", " << it.value().toString().toLocal8Bit().data() << " ) ";
    }
    stream << ")";
    return stream;
}

static inline std::ostream& operator<<( std::ostream& stream, const QSize& size )
{
    stream << "QSize( " << size.width() << "x" << size.height() << " )";
    return stream;
}

static inline std::ostream& operator<<( std::ostream& stream, const QSizeF& size )
{
    stream << "QSizeF( " << size.width() << "x" << size.height() << " )";
    return stream;
}

static inline std::ostream& operator<<( std::ostream& stream, const QRect& rect )
{
    stream << "QRect( " << rect.topLeft() << ", " << rect.size() << " )";
    return stream;
}

static inline std::ostream& operator<<( std::ostream& stream, const QRectF& rect )
{
    stream << "QRectF( " << rect.topLeft() << ", " << rect.size() << " )";
    return stream;
}

#ifndef QT_NO_REGEXP
static inline std::ostream& operator<<( std::ostream& stream, const QRegExp& r )
{
    stream << "QRegExp( " << r.pattern().toLocal8Bit().data() << " )";
    return stream;
}
#endif

#if 0
static inline std::ostream& operator<<( std::ostream& stream, const QString& string )
{
    stream << "QString( \"" << string.toLocal8Bit().data() << "\" )";
    return stream;
}
#endif

static inline std::ostream& operator<<( std::ostream& stream, const QStringList& list )
{
    stream << "QStringList(";
    for( QStringList::const_iterator it = list.begin(); it != list.end(); ++it )
    {
        stream << " " << it->toLocal8Bit().data();
        if( it + 1 != list.end() )
            stream << ",";
    }
    stream << " )";
    return stream;
}

static inline std::ostream& operator<<( std::ostream& stream, const QTime& d )
{
    stream << "QTime( " << d.toString().toLocal8Bit().data() << " )";
    return stream;
}

#if 0
static inline std::ostream& operator<<( std::ostream& stream, const QUrl& url )
{
    stream << "QUrl( \"" << url.toString().toLocal8Bit().data() << "\" )";
    return stream;
}
#endif

static inline std::ostream& operator<<( std::ostream& stream, const QVariant& url )
{
    stream << "QVariant( " << url.toString().toLocal8Bit().data() << " )";
    return stream;
}
QT_END_NAMESPACE

Q_DECLARE_METATYPE( _qvariant_converter_uniq_1389392::MyClass )
Q_DECLARE_METATYPE( _qvariant_converter_uniq_1389392::MyClass2 )

KDAB_UNITTEST_SIMPLE( KDVariantConverter, "kdtools/core" ) {

    KDVariantConverter::registerConversion< MyClass, QVariant::BitArray >();
    KDVariantConverter::registerConversion< MyClass, QVariant::Bool >();
    KDVariantConverter::registerConversion< MyClass, QVariant::ByteArray >();
    KDVariantConverter::registerConversion< MyClass, QVariant::Char >();
    KDVariantConverter::registerConversion< MyClass, QVariant::Date >();
    KDVariantConverter::registerConversion< MyClass, QVariant::DateTime >();
    KDVariantConverter::registerConversion< MyClass, QVariant::Double >();
    KDVariantConverter::registerConversion< MyClass, QVariant::Int >();
    KDVariantConverter::registerConversion< MyClass, QVariant::Line >();
    KDVariantConverter::registerConversion< MyClass, QVariant::LineF >();
    KDVariantConverter::registerConversion< MyClass, QVariant::List >();
    KDVariantConverter::registerConversion< MyClass, QVariant::Locale >();
    KDVariantConverter::registerConversion< MyClass, QVariant::LongLong >();
    KDVariantConverter::registerConversion< MyClass, QVariant::Map >();
    KDVariantConverter::registerConversion< MyClass, QVariant::Point >();
    KDVariantConverter::registerConversion< MyClass, QVariant::PointF >();
    KDVariantConverter::registerConversion< MyClass, QVariant::Rect >();
    KDVariantConverter::registerConversion< MyClass, QVariant::RectF >();
    KDVariantConverter::registerConversion< MyClass, QVariant::RegExp >();
    KDVariantConverter::registerConversion< MyClass, QVariant::Size >();
    KDVariantConverter::registerConversion< MyClass, QVariant::SizeF >();
    KDVariantConverter::registerConversion< MyClass >(); // this is QString
    KDVariantConverter::registerConversion< MyClass2 >(); // this is QString
    KDVariantConverter::registerConversion< MyClass, QVariant::StringList >();
    KDVariantConverter::registerConversion< MyClass, QVariant::Time >();
    KDVariantConverter::registerConversion< MyClass, QVariant::UInt >();
    KDVariantConverter::registerConversion< MyClass, QVariant::ULongLong >();
    KDVariantConverter::registerConversion< MyClass, QVariant::Url >();

    {
    MyClass m;
    const QVariant var = qVariantFromValue< MyClass >( m );

    bool ok1;
    bool ok2;
    assertEqual( m.toBitArray(), var.toBitArray() );
    assertEqual( m.toBool(), var.toBool() );
    assertEqual( m.toByteArray(), var.toByteArray() );
    assertEqual( m.toChar(), var.toChar() );
    assertEqual( m.toDate(), var.toDate() );
    assertEqual( m.toDateTime(), var.toDateTime() );
    assertEqual( m.toDouble( &ok1 ), var.toDouble( &ok2 ) );         assertTrue( ok1 ); assertTrue( ok2 );
    assertEqual( m.toInt( &ok1 ), var.toInt( &ok2 ) );               assertTrue( ok1 ); assertTrue( ok2 );
#ifndef QT_NO_GEOM_VARIANT
    assertEqual( m.toLine(), var.toLine() );
    assertEqual( m.toLineF(), var.toLineF() );
#endif
    assertEqual( m.toList(), var.toList() );
    assertEqual( m.toLocale(), var.toLocale() );
    assertEqual( m.toLongLong( &ok1 ), var.toLongLong( &ok2 ) );     assertTrue( ok1 ); assertTrue( ok2 );
    assertEqual( m.toMap(), var.toMap() );
#ifndef QT_NO_GEOM_VARIANT
    assertEqual( m.toPoint(), var.toPoint() );
    assertEqual( m.toPointF(), var.toPointF() );
    assertEqual( m.toRect(), var.toRect() );
    assertEqual( m.toRectF(), var.toRectF() );
#endif
#ifndef QT_NO_REGEXP
    assertEqual( m.toRegExp(), var.toRegExp() );
#endif
#ifndef QT_NO_GEOM_VARIANT
    assertEqual( m.toSize(), var.toSize() );
    assertEqual( m.toSizeF(), var.toSizeF() );
#endif
    assertEqual( m.toString(), var.toString() );
    assertEqual( m.toStringList(), var.toStringList() );
    assertEqual( m.toTime(), var.toTime() );
    assertEqual( m.toUInt( &ok1 ), var.toUInt( &ok2 ) );             assertTrue( ok1 ); assertTrue( ok2 );
    assertEqual( m.toULongLong( &ok1 ), var.toULongLong( &ok2 ) );   assertTrue( ok1 ); assertTrue( ok2 );
    assertEqual( m.toUrl(), var.toUrl() );
    }

    {
    MyClass m1a;
    MyClass m1b;
    MyClass2 m2a( 4 );
    MyClass2 m2b( 5 );
    MyClass2 m2c( 4 );
    const QVariant var1a = qVariantFromValue< MyClass >( m1a );
    const QVariant var1b = qVariantFromValue< MyClass >( m1b );
    const QVariant var2a = qVariantFromValue< MyClass2 >( m2a );
    const QVariant var2b = qVariantFromValue< MyClass2 >( m2b );
    const QVariant var2c = qVariantFromValue< MyClass2 >( m2c );
    assertFalse( var1a < var1b );      // operator< not defined -> string comparison (toString() is always the same value )
    assertNotEqual( var1a, var2a );   // operator== not defined for MyClass -> pointer comparison -> false

    assertTrue( var2a < var2b );     // operator< is defined
    assertTrue( var2a <= var2b );
    assertFalse( var2a >= var2b );
    assertFalse( var2a == var2b );
    assertTrue( var2a >= var2c );
    assertTrue( var2a <= var2c );
    assertTrue( var2a < var2b );
    assertTrue( var2b > var2a );
    assertTrue( var2a != var2b );
    assertNotEqual( var2a, var2b );      // operator== is defined
    assertEqual( var2a, var2c );
    }

    {
    const QVariant var1 = QString::fromLatin1( "foo" );
    const QVariant var2 = QString::fromLatin1( "bar" );
    assertTrue( var1 > var2 );    // this will do string conversion
    assertFalse( var1 < var2 );
    }

    {
    const QVariant var1 = 1;
    const QVariant var2 = 2;
    assertTrue( var1 < var2 );
    assertFalse( var1 > var2 );
    }
}

#endif

#endif
