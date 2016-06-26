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

#ifndef __KDTOOLSCORE_KDTHREADRUNNER_H__
#define __KDTOOLSCORE_KDTHREADRUNNER_H__

#include <QMutexLocker>
#include <QtCore/QThread>

#include <KDToolsCore/pimpl_ptr.h>

class KDTOOLSCORE_EXPORT KDThreadRunnerBase : public QThread
{
public:
    ~KDThreadRunnerBase();

protected:
    explicit KDThreadRunnerBase( QObject* parent = 0 );
    void doStart( Priority prio );
    void doExec();

    void setImpl( QObject* );
    QObject* impl() const;

    QMutex* internalStartThreadMutex();

private:
    class Private;
    kdtools::pimpl_ptr< Private > d;
};

template <class T>
class MAKEINCLUDES_EXPORT KDThreadRunner
#ifdef DOXYGEN_RUN
 : public QThread
#else
 : public KDThreadRunnerBase
#endif
{
public:
    explicit KDThreadRunner( QObject * p=0 )
        : KDThreadRunnerBase( p ) {}

    T * startThread( Priority prio = InheritPriority ) {
        QMutexLocker locker( this->internalStartThreadMutex() );
        if ( !this->impl() ) {
            this->doStart( prio );
        }
        return static_cast<T*>( this->impl() );
    }

protected:
    void run() KDAB_OVERRIDE // KDAB_FINAL?
    {
        // impl is created in the thread so that m_impl->thread()==this
        T t(this);
        this->setImpl( &t );
        this->doExec();
        this->setImpl( 0 );
    }
};

#endif
