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

#ifndef __KDTOOLSCORE__KDGENERICFACTORY_H__
#define __KDTOOLSCORE__KDGENERICFACTORY_H__

#include <KDToolsCore/kdtoolsglobal.h>

#include <QtCore/QHash>
#include <QtCore/QMap>
#include <QtCore/QString>

#ifndef DOXYGEN_RUN

template< typename T_Identifier, template< typename U, typename V > class T_Map >
class KDTOOLSCORE_EXPORT KDGenericFactoryBase
{
protected:
    ~KDGenericFactoryBase() {}

    typedef void* (*FactoryFunction)();

    void unregisterProduct( const T_Identifier& name )
    {
        map.remove( name );
    }

    unsigned int productCount() const
    {
        return map.size();
    }

    QList< T_Identifier > availableProducts() const
    {
        return map.keys();
    }

    void * create( const T_Identifier & name ) const
    {
        const typename T_Map< T_Identifier, FactoryFunction >::const_iterator it = map.find( name );
        if( it == map.end() )
            return 0;
        return (*it)();
    }

protected:
    void registerProductionFunction( const T_Identifier& name, FactoryFunction createfun )
    {
        map.insert( name, createfun );
    }

private:
    T_Map< T_Identifier, FactoryFunction > map;
};

#ifdef KDAB_HAVE_CPP11_EXTERN_TEMPLATES
extern template class KDGenericFactoryBase<QString,QHash>;
extern template class KDGenericFactoryBase<QString,QMap>;
extern template class KDGenericFactoryBase<int,QHash>;
extern template class KDGenericFactoryBase<int,QMap>;
#endif

#endif // DOXYGEN_RUN

template< typename T_Product, typename T_Identifier = QString, template< typename U, typename V > class T_Map = QHash >
class MAKEINCLUDES_EXPORT KDGenericFactory
#ifndef DOXYGEN_RUN
 : KDGenericFactoryBase<T_Identifier,T_Map>
#endif
{
    typedef KDGenericFactoryBase<T_Identifier,T_Map> base;
public:
    virtual ~KDGenericFactory()
    {
    }

    typedef T_Product* (*FactoryFunction)();

    template< typename T >
    void registerProduct( const T_Identifier& name )
    {
        FactoryFunction function = &KDGenericFactory::template create<T>; // disambiguate
        registerProductionFunction( name, function );
    }

#ifdef DOXYGEN_RUN
    void unregisterProduct( const T_Identifier& name );
    unsigned int productCount() const;
    QList< T_Identifier > availableProducts() const;
#else
    using base::unregisterProduct;
    using base::productCount;
    using base::availableProducts;
#endif

    T_Product* create( const T_Identifier& name ) const
    {
        return static_cast<T_Product*>( base::create( name ) );
    }

protected:
    void registerProductionFunction( const T_Identifier& name, FactoryFunction createfun )
    {
        typename base::FactoryFunction fun = reinterpret_cast<typename base::FactoryFunction>( createfun );
        base::registerProductionFunction( name, fun );
    }

private:
    template< typename T >
    static T_Product * create()
    {
        return new T;
    }

};

#endif
