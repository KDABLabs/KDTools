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

#ifndef __KDTOOLS_KDUPDATERUPDATEOPERATION_H__
#define __KDTOOLS_KDUPDATERUPDATEOPERATION_H__

#include "kdupdater.h"
#include <KDToolsCore/pimpl_ptr.h>

#include "kdupdaterupdateoperationfactory.h"

#ifndef KDTOOLS_NO_COMPAT
# include "kdupdaterapplication.h"
#endif // KDTOOLS_NO_COMPAT

#include <QtCore/QCoreApplication>
#include <QtCore/QVariant>
#include <QtCore/QStringList>

QT_BEGIN_NAMESPACE
class QDomDocument;
QT_END_NAMESPACE

namespace KDUpdater
{
    class Target;

    class KDTOOLS_UPDATER_EXPORT UpdateOperation
    {
        Q_DECLARE_TR_FUNCTIONS(UpdateOperation)

        friend class KDUpdater::UpdateOperationFactory;

    public:
        enum Error {
            NoError=0,
            InvalidArguments=1,
            UserDefinedError=128
        };

        UpdateOperation();
        virtual ~UpdateOperation();

        QString name() const;
        QString operationCommand() const;

        QStringList arguments() const;
        QString errorString() const;
        int error() const;

        virtual void backup() = 0;
        virtual bool performOperation() = 0;
        virtual bool undoOperation() = 0;
        virtual bool testOperation() = 0;
        virtual UpdateOperation* clone() const = 0;

        QString lastError() const;

        virtual QDomDocument toXml() const;
        bool fromXml( const QString &xml );
        virtual bool fromXml( const QDomDocument &doc );

    protected:
        template< class T >
        T* clone() const
        {
            T* const cloned = new T;
            cloned->setValues( values() );
            cloned->setArguments( arguments() );
            return cloned;
        }

        bool hasValue( const QString& name ) const;
        void clearValue( const QString& name );
        QVariant value( const QString& name ) const;
        void setValue( const QString& name, const QVariant &value );

        void setArguments(const QStringList& args);
        void setTarget( Target * target );
  
        void setName(const QString& name);
        Target * target() const;
        void setErrorString( const QString& errorString );
        void setError( int error );
        void setError( int error, const QString& errorString );

#ifndef KDTOOLS_NO_COMPAT
        void setApplication( Application * a ) { setTarget( a ); }
        Application * application() const { return dynamic_cast<Application*>( target() ); }
#endif // KDTOOLS_NO_COMPAT

    private:
        QVariantMap values() const;
        void setValues( const QVariantMap& values );

        class Private;
        kdtools::pimpl_ptr< Private > d;
    };

}

#endif
