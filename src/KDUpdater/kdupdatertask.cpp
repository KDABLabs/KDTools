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

#include "kdupdatertask.h"

#include "kdupdatertarget.h"

/*!
   \ingroup kdupdater
   \class KDUpdater::Task kdupdatertask.h KDUpdaterTask
   \brief Base class for all task classes in KDUpdater

   This class is the base class for all task classes in KDUpdater. Task is an activity that
   occupies certain amount of execution time. It can be started, stopped (or canceled), paused and
   resumed. Tasks can report progress and error messages which an application can show in any
   sort of UI. The KDUpdater::Task class provides a common interface for dealing with all kinds of
   tasks in KDUpdater. The class diagram show in this class documentation will help in pointing out
   the task classes in KDUpdater.

   User should be carefull of these points:
   \li Instances of this class cannot be created. Only instance of the subclasses can be created
   \li Task classes can be started only once.
*/

using namespace KDUpdater;

class Task::Private
{
public:
    Private( Task* qq) :
        q( qq ),
        caps( Task::NoCapability ),
        errorCode( 0 ),
        started( false ),
        finished( false ),
        paused( false ),
        stopped( false ),
        progressPc( 0 )
    {
    }

    Task* q;
    int caps;
    QString name;
    int errorCode;
    QString errorText;
    bool started;
    bool finished;
    bool paused;
    bool stopped;
    int progressPc;
    QString progressText;
};

/*!
   \internal
*/
Task::Task( const QString & name, int caps, Target * target )
    : QObject(),
      d( new Private( this ) )
{
    if ( target )
        target->addTask( this );
    d->caps = caps;
    d->name = name;
}

/*!
   \internal
*/
Task::~Task()
{
}

/*!
   Contains the name of the task.

   Read this property's value using %name().
*/
QString Task::name() const
{
    return d->name;
}

/*!
   Contains the capabilities of the task. It is a combination of one or more
   Capability flags. Defined as follows
   \code
   enum KDUpdater::Task::Capability
   {
       NoCapability	= 0,
       Pausable     = 1,
       Stoppable    = 2
   };
   \endcode

   Read this property's value using %capabilities().
*/
int Task::capabilities() const
{
    return d->caps;
}

/*!
   Contains the last reported error code.

   Read this property's value using %error().
*/
int Task::error() const
{
    return d->errorCode;
}

/*!
   Contains the last reported error message text.

   Read this property's value using %errorString().
*/
QString Task::errorString() const
{
    return d->errorText;
}

/*!
   Returns whether the task has started and is running or not.
*/
bool Task::isRunning() const
{
    return d->started;
}

/*!
   Returns whether the task has finished or not.

   \note Stopped (or canceled) tasks are not finished tasks.
*/
bool Task::isFinished() const
{
    return d->finished;
}

/*!
   Returns whether the task is paused or not.
*/
bool Task::isPaused() const
{
    return d->paused;
}

/*!
   Returns whether the task is stopped or not.

   \note Finished tasks are not stopped classes.
*/
bool Task::isStopped() const
{
    return d->stopped;
}

/*!
   Contains the progress in percentage made by this task.

   Read this property's value using %progressPercent().
*/
int Task::progressPercent() const
{
    return d->progressPc;
}

/*!
   Contains a string that describes the progress made by this task as a string.

   Read this property's value using %progressText().
*/
QString Task::progressText() const
{
    return d->progressText;
}

/*!
   Starts the task.
*/
void Task::run()
{
    if(d->started)
    {
        qDebug("Trying to start an already started task");
        return;
    }

    if(d->finished || d->stopped)
    {
        qDebug("Trying to start a finished or canceled task");
        return;
    }

    d->stopped = false;
    d->finished = false; // for the sake of completeness
    d->started = true;
    emit started();
    reportProgress(0, tr("%1 started").arg(d->name));

    doRun();
}

/*!
   Stops the task, provided the task has \ref Stoppable capability.

   \note Once the task is stopped, it cannot be restarted.
*/
void Task::stop()
{
    if( !(d->caps & Stoppable) )
    {
        const QString errorMsg = tr("'%1' cannot be stopped").arg(d->name);
        reportError(ECannotStopTask, errorMsg);
        return;
    }

    if(!d->started)
    {
        qDebug("Trying to stop an unstarted task");
        return;
    }

    if(d->finished || d->stopped)
    {
        qDebug("Trying to stop a finished or canceled task");
        return;
    }

    d->stopped = doStop();
    if(!d->stopped)
    {
        const QString errorMsg = tr("Cannot stop task '%1'").arg(d->name);
        reportError(ECannotStopTask, errorMsg);
        return;
    }

    d->started = false; // the task is not running
    d->finished = false; // the task is not finished, but was canceled half-way through

    emit stopped();
}

/*!
   Paused the task, provided the task has \ref Pausable capability.
*/
void Task::pause()
{
    if( !(d->caps & Pausable) )
    {
        const QString errorMsg = tr("'%1' cannot be paused").arg(d->name);
        reportError(ECannotPauseTask, errorMsg);
        return;
    }

    if(!d->started)
    {
        qDebug("Trying to pause an unstarted task");
        return;
    }

    if(d->finished || d->stopped)
    {
        qDebug("Trying to pause a finished or canceled task");
        return;
    }

    d->paused = doPause();
    if(!d->paused)
    {
        const QString errorMsg = tr("Cannot pause task '%1'").arg(d->name);
        reportError(ECannotPauseTask, errorMsg);
        return;
    }

    // The task state has to be started, paused but not finished or stopped.
    // We need not set the flags below, but just in case.
    // Perhaps we should do Q_ASSERT() ???
    d->started = true;
    d->finished = false;
    d->stopped = false;

    emit paused();
}

/*!
   Resumes the task if it was paused.
*/
void Task::resume()
{
    if(!d->paused)
    {
        qDebug("Trying to resume an unpaused task");
        return;
    }

    const bool val = doResume();
    if(!val)
    {
        const QString errorMsg = tr("Cannot resume task '%1'").arg(d->name);
        reportError(ECannotResumeTask, errorMsg);
        return;
    }

    // The task state should be started, but not paused, finished or stopped.
    // We need not set the flags below, but just in case.
    // Perhaps we should do Q_ASSERT() ???
    d->started = true;
    d->paused = false;
    d->finished = false;
    d->stopped = false;

    emit resumed();
}

/*!
   \internal
*/
void Task::reportProgress(int percent, const QString& text)
{
    if(d->progressPc == percent)
        return;

    d->progressPc = percent;
    d->progressText = text;
    emit progressValue( d->progressPc );
    emit progressText( d->progressText );
}

/*!
   \internal
*/
void Task::reportError(int errorCode, const QString& errorText)
{
    d->errorCode = errorCode;
    d->errorText = errorText;

    emit error(d->errorCode, d->errorText);
}

/*!
   \internal
*/
void Task::reportDone()
{
    const QString msg = tr("%1 done");
    reportProgress(100, msg);

    // State should be finished, but not started, paused or stopped.
    d->finished = true;
    d->started = false;
    d->paused = false;
    d->stopped = false;
    d->errorCode = 0;
    d->errorText.clear();

    emit finished();
}

/*!
   \fn virtual bool KDUpdater::Task::doStart() = 0;
*/

/*!
   \fn virtual bool KDUpdater::Task::doStop() = 0;
*/

/*!
   \fn virtual bool KDUpdater::Task::doPause() = 0;
*/

/*!
   \fn virtual bool KDUpdater::Task::doResume() = 0;
*/

/*!
   \signal void KDUpdater::Task::error(int code, const QString& errorText)

   This signal is emitted to notify an error during the execution of this task.
   \param code Error code
   \param errorText A string describing the error.

   Error codes are just integers, there are however built in errors represented
   by the KDUpdater::Error enumeration
   \code
   enum Error
   {
   ECannotStartTask,
   ECannotPauseTask,
   ECannotResumeTask,
   ECannotStopTask,
   EUnknown
   };
   \endcode
*/

/*!
   \signal void KDUpdater::Task::progress(int percent, const QString& progressText)

   This signal is emitted to nofity progress made by the task.

   \param percent Percentage of progress made
   \param progressText A string describing the progress made
*/

/*!
   \signal void KDUpdater::Task::started()

   This signal is emitted when the task has started.
*/

/*!
   \signal void KDUpdater::Task::paused()

   This signal is emitted when the task has paused.
*/

/*!
   \signal void KDUpdater::Task::resumed()

   This signal is emitted when the task has resumed.
*/

/*!
   \signal void KDUpdater::Task::stopped()

   This signal is emitted when the task has stopped (or canceled).
*/

/*!
   \signal void KDUpdater::Task::finished()

   This signal is emitted when the task has finished.
*/
