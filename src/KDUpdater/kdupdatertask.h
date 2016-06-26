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

#ifndef __KDTOOLS_KDUPDATERTASK_H__
#define __KDTOOLS_KDUPDATERTASK_H__

#include "kdupdater.h"
#include <KDToolsCore/pimpl_ptr.h>

#include <QtCore/QObject>

namespace KDUpdater
{

    class Target;

    class KDTOOLS_UPDATER_EXPORT Task : public QObject
    {
        Q_OBJECT
        Q_PROPERTY( int capabilities READ capabilities )
        Q_PROPERTY( int error READ error )
        Q_PROPERTY( QString errorString READ errorString )
        Q_PROPERTY( QString name READ name )
        Q_PROPERTY( int progressPercent READ progressPercent )
        Q_PROPERTY( QString progressText READ progressText )

    public:
        enum Capability
        {
            NoCapability = 0,
            Pausable     = 1,
            Stoppable    = 2
        };

        virtual ~Task();

        QString name() const;
        int capabilities() const;

        int error() const;
        QString errorString() const;

        bool isRunning() const;
        bool isFinished() const;
        bool isPaused() const;
        bool isStopped() const;

        int  progressPercent() const;
        QString progressText() const;

    public Q_SLOTS:
        void run();
        void stop();
        void pause();
        void resume();

    Q_SIGNALS:
        void error(int code, const QString& errorText);
        void progressValue(int percent);
        void progressText(const QString& progressText);
        void started();
        void paused();
        void resumed();
        void stopped();
        void finished();

    protected:
        explicit Task( const QString & name, int caps=NoCapability, Target * target=0 );
        void reportProgress(int percent, const QString& progressText);
        void reportError(int errorCode, const QString& errorText);
        void reportDone();

        void reportError(const QString& errorText)
        {
            reportError(EUnknown, errorText);
        }

    protected:
        // Task interface
        virtual void doRun() = 0;
        virtual bool doStop() = 0;
        virtual bool doPause() = 0;
        virtual bool doResume() = 0;

    private:
        class Private;
        kdtools::pimpl_ptr< Private > d;
    };
}

#endif
