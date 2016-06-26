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

#ifndef __KDTOOLSCORE__KDVARIANTCONVERTER_H__
#define __KDTOOLSCORE__KDVARIANTCONVERTER_H__

#include <KDToolsCore/kdtoolsglobal.h>

#if QT_VERSION >= 0x040200 || defined( DOXYGEN_RUN )

#include <QtCore/QVariant>
#include <map>

namespace kdtools {

struct comparator_function
{
    comparator_function();

    operator bool() const;
    bool operator()( const QVariant::Private* a, const QVariant::Private* b ) const;

protected:
    comparator_function( QVariant::f_compare function );

private:
    QVariant::f_compare function;
};

struct converter_function
{
    converter_function();

    operator bool() const;
#if QT_VERSION >= 0x050000
    bool operator()( const QVariant::Private* d, int t, void* result, bool* ok ) const;
#else
    bool operator()( const QVariant::Private* d, QVariant::Type t, void* result, bool* ok ) const;
#endif

protected:
    converter_function( QVariant::f_convert function );

private:
    QVariant::f_convert function;
};

class qvariant_conversion_registrar : public QVariant
{
private:
    qvariant_conversion_registrar();

public:
    static void registerIt();

    struct converter
    {
#if QT_VERSION >= 0x050000
        converter( int id = 0, int targetType = QVariant::String );

        int id;
        int targetType;
#else
        converter( int id = 0, QVariant::Type targetType = QVariant::String );

        int id;
        QVariant::Type targetType;
#endif
    };

    static bool registered;

    static std::map< converter, converter_function >* converters;
    static std::map< int, comparator_function >* comparators_equals;
    static std::map< int, comparator_function >* comparators_less_than;
};

bool operator<( const qvariant_conversion_registrar::converter& lhs, const qvariant_conversion_registrar::converter& rhs );

template< typename T >
const T* valueHelper( const QVariant::Private* d )
{
    return reinterpret_cast< const T* >( d->is_shared ? d->data.shared->ptr : &d->data.ptr );
}

namespace fallback_comparator
{
    template< typename T >
    bool operator==( const T& lhs, const T& rhs )
    {
        qWarning( "Warning: No operator== defined for type \"%s\".", QMetaType::typeName( qMetaTypeId< T >() ) );
        return &lhs == &rhs;
    }

    template< typename T >
    bool operator<( const T& lhs, const T& rhs )
    {
        return qVariantFromValue< T >( lhs ).toString() < qVariantFromValue< T >( rhs ).toString();
    }
}

template< typename T >
bool equals( const QVariant::Private* a, const QVariant::Private* b )
{
    using namespace fallback_comparator;

    const T* const A = valueHelper< T >( a );
    const T* const B = valueHelper< T >( b );
    if( A == B )
        return true;
    else if( A == 0 || B == 0 )
        return false;
    else
        return *A == *B;
}

template< typename T >
bool less_than( const QVariant::Private* a, const QVariant::Private* b )
{
    using namespace fallback_comparator;

    const T* const A = valueHelper< T >( a );
    const T* const B = valueHelper< T >( b );
    if( A == B )
        return false;
    else if( A == 0 || B == 0 )
        return A < B;
    else
        return *A < *B;
}

template< typename T >
struct comparator_equals : public comparator_function
{
    comparator_equals< T >()
        : comparator_function( equals< T > )
    {
    }
};

template< typename T >
struct comparator_less_than : public comparator_function
{
    comparator_less_than< T >()
        : comparator_function( less_than< T > )
    {
    }
};

template< typename T, QVariant::Type TARGET >
struct converter : public converter_function
{
};

#define CONVERTER_CHECK_INPUT( VARIANT_TYPE )                                                         \
    Q_ASSERT( t == QVariant::VARIANT_TYPE );                                                          \
    Q_UNUSED( t )                                                                                     \
    const T* const v = valueHelper< T >( d );                                                         \
    if( v == 0 ) { if( ok ) *ok = false; return false; }                                                                                                 


#if QT_VERSION >= 0x050000
#define CONVERTER_FUNCTIONS_TEMPLATE( VARIANT_TYPE, TYPE )                                            \
template< typename T >                                                                                \
bool doConvertTo##VARIANT_TYPE( const T* v, void* result, bool* ok )                                  \
{                                                                                                     \
    TYPE* const str = static_cast< TYPE* >( result );                                                 \
    *str = v->T::to##VARIANT_TYPE();                                                                  \
    if( ok )                                                                                          \
        *ok = true;                                                                                   \
    return true;                                                                                      \
}                                                                                                     \
                                                                                                      \
template< typename T >                                                                                \
bool doConvertTo##VARIANT_TYPE##Ok( const T* v, void* result, bool* ok )                              \
{                                                                                                     \
    TYPE* const str = static_cast< TYPE* >( result );                                                 \
    *str = v->T::to##VARIANT_TYPE( ok );                                                              \
    if( ok )                                                                                          \
        return *ok;                                                                                   \
    return true;                                                                                      \
}                                                                                                     \
                                                                                                      \
template< typename T >                                                                                \
bool convertTo##VARIANT_TYPE( const QVariant::Private* d, int t, void* result, bool* ok )             \
{                                                                                                     \
    CONVERTER_CHECK_INPUT( VARIANT_TYPE )                                                             \
    return doConvertTo##VARIANT_TYPE< T >( v, result, ok );                                           \
}                                                                                                     \
                                                                                                      \
template< typename T >                                                                                \
bool convertOkTo##VARIANT_TYPE( const QVariant::Private* d, int t, void* result, bool* ok )           \
{                                                                                                     \
    CONVERTER_CHECK_INPUT( VARIANT_TYPE )                                                             \
    return doConvertTo##VARIANT_TYPE##Ok< T >( v, result, ok );                                       \
}

#else

#define CONVERTER_FUNCTIONS_TEMPLATE( VARIANT_TYPE, TYPE )                                            \
template< typename T >                                                                                \
bool doConvertTo##VARIANT_TYPE( const T* v, void* result, bool* ok )                                  \
{                                                                                                     \
    TYPE* const str = static_cast< TYPE* >( result );                                                 \
    *str = v->T::to##VARIANT_TYPE();                                                                  \
    if( ok )                                                                                          \
        *ok = true;                                                                                   \
    return true;                                                                                      \
}                                                                                                     \
                                                                                                      \
template< typename T >                                                                                \
bool doConvertTo##VARIANT_TYPE##Ok( const T* v, void* result, bool* ok )                              \
{                                                                                                     \
    TYPE* const str = static_cast< TYPE* >( result );                                                 \
    *str = v->T::to##VARIANT_TYPE( ok );                                                              \
    if( ok )                                                                                          \
        return *ok;                                                                                   \
    return true;                                                                                      \
}                                                                                                     \
                                                                                                      \
template< typename T >                                                                                \
bool convertTo##VARIANT_TYPE( const QVariant::Private* d, QVariant::Type t, void* result, bool* ok )  \
{                                                                                                     \
    CONVERTER_CHECK_INPUT( VARIANT_TYPE )                                                             \
    return doConvertTo##VARIANT_TYPE< T >( v, result, ok );                                           \
}                                                                                                     \
                                                                                                      \
template< typename T >                                                                                \
bool convertOkTo##VARIANT_TYPE( const QVariant::Private* d, QVariant::Type t, void* result, bool* ok )\
{                                                                                                     \
    CONVERTER_CHECK_INPUT( VARIANT_TYPE )                                                             \
    return doConvertTo##VARIANT_TYPE##Ok< T >( v, result, ok );                                       \
}
#endif


#define CONVERTER_TEMPLATE( VARIANT_TYPE, TYPE )                                                      \
CONVERTER_FUNCTIONS_TEMPLATE( VARIANT_TYPE, TYPE )                                                    \
template< typename T >                                                                                \
struct converter< T, QVariant::VARIANT_TYPE > : public converter_function                             \
{                                                                                                     \
    converter< T, QVariant::VARIANT_TYPE >()                                                          \
        : converter_function( convertTo##VARIANT_TYPE< T > )                                          \
    {                                                                                                 \
    }                                                                                                 \
};


#define CONVERTER_TEMPLATE_OK( VARIANT_TYPE, TYPE )                                                   \
CONVERTER_FUNCTIONS_TEMPLATE( VARIANT_TYPE, TYPE )                                                    \
template< typename T >                                                                                \
struct converter< T, QVariant::VARIANT_TYPE > : public converter_function                             \
{                                                                                                     \
    converter< T, QVariant::VARIANT_TYPE >()                                                          \
        : converter_function( convertOkTo##VARIANT_TYPE< T > )                                        \
    {                                                                                                 \
    }                                                                                                 \
};

// without bool* ok = 0
CONVERTER_TEMPLATE( BitArray, QBitArray )
CONVERTER_TEMPLATE( Bool, bool )
CONVERTER_TEMPLATE( ByteArray, QByteArray )
CONVERTER_TEMPLATE( Char, QChar )
CONVERTER_TEMPLATE( Date, QDate )
CONVERTER_TEMPLATE( DateTime, QDateTime )
CONVERTER_TEMPLATE( List, QVariantList )
CONVERTER_TEMPLATE( Locale, QLocale )
CONVERTER_TEMPLATE( Map, QVariantMap )
#ifndef QT_NO_REGEXP
CONVERTER_TEMPLATE( RegExp, QRegExp )
#endif
#ifndef QT_NO_GEOM_VARIANT
CONVERTER_TEMPLATE( Line, QLine )
CONVERTER_TEMPLATE( LineF, QLineF )
CONVERTER_TEMPLATE( Size, QSize )
CONVERTER_TEMPLATE( SizeF, QSizeF )
CONVERTER_TEMPLATE( Rect, QRect )
CONVERTER_TEMPLATE( RectF, QRectF )
CONVERTER_TEMPLATE( Point, QPoint )
CONVERTER_TEMPLATE( PointF, QPointF )
#endif
CONVERTER_TEMPLATE( String, QString )
CONVERTER_TEMPLATE( StringList, QStringList )
CONVERTER_TEMPLATE( Time, QTime )
CONVERTER_TEMPLATE( Url, QUrl )

// with bool* ok = 0
CONVERTER_TEMPLATE_OK( Double, double )
CONVERTER_TEMPLATE_OK( Int, int )
CONVERTER_TEMPLATE_OK( LongLong, qlonglong )
CONVERTER_TEMPLATE_OK( UInt, uint )
CONVERTER_TEMPLATE_OK( ULongLong, qulonglong )

} // namespace kdtools

class KDTOOLSCORE_EXPORT KDVariantConverter
{
public:
    template< typename TYPE, QVariant::Type VARIANT_TYPE >
    static int registerConversion()
    {
        return registerConversion< TYPE, VARIANT_TYPE >( kdtools::converter< TYPE, VARIANT_TYPE >() );
    }

    template< typename TYPE >
    static int registerConversion()
    {
        return registerConversion< TYPE, QVariant::String >();
    }

protected:
    template< typename TYPE, QVariant::Type VARIANT_TYPE >
    static int registerConversion( const kdtools::converter_function& converter )
    {
        kdtools::qvariant_conversion_registrar::registerIt();
        const int metaTypeId = QMetaTypeId< TYPE >::qt_metatype_id();
        (*kdtools::qvariant_conversion_registrar::converters)[ kdtools::qvariant_conversion_registrar::converter( metaTypeId, VARIANT_TYPE ) ] = converter;
        (*kdtools::qvariant_conversion_registrar::comparators_equals)[ metaTypeId ] = kdtools::comparator_equals< TYPE >();
        (*kdtools::qvariant_conversion_registrar::comparators_less_than)[ metaTypeId ] = kdtools::comparator_less_than< TYPE >();
        return metaTypeId;
    }

private:
    KDVariantConverter();
};


bool operator<( const QVariant& lhs, const QVariant& rhs );
static inline bool operator>( const QVariant& lhs, const QVariant& rhs ) { return lhs != rhs && !operator<( lhs, rhs ); }
static inline bool operator<=( const QVariant& lhs, const QVariant& rhs ) { return !operator>( lhs, rhs ); }
static inline bool operator>=( const QVariant& lhs, const QVariant& rhs ) { return !operator<( lhs, rhs ); }

#endif

#endif
