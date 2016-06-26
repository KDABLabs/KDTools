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

#include <KDToolsCore/KDThreadRunner>
#include <QPushButton>
#include <QApplication>
#include <QTimer>
#include <QDebug>

class ThreadImpl : public QObject
{
    Q_OBJECT
public:
    explicit ThreadImpl(KDThreadRunner<ThreadImpl>* runner)
        : QObject(),
          // m_runner is optional, only needed if the thread implementation
          // needs a pointer to the KDThreadRunner.
          m_runner(runner),
          timer(new QTimer(this))
    {
        connect(timer, SIGNAL(timeout()), this, SLOT(slotTimeout()));
        timer->start(200);
        Q_ASSERT(thread() == m_runner);
    }

private Q_SLOTS:
    void slotTimeout()
    {
        qDebug() << "timeout called in thread" << QThread::currentThread();
        Q_ASSERT(QThread::currentThread() == m_runner);
    }
private:
    KDThreadRunner<ThreadImpl>* m_runner;
    QTimer* timer;
};

class TestButton : public QPushButton
{
    Q_OBJECT
public:
    TestButton(ThreadImpl* impl) : QPushButton(tr("Stop thread"), 0), m_impl(impl) {
        connect(this, SIGNAL(clicked()), this, SLOT(slotQuit()));
    }

public Q_SLOTS:
    void slotQuit()
    {
        qDebug() << "slotQuit";
        // Note: quit() didn't work with Qt-4.2.3 on Solaris, so there we had to use:
        // m_impl->thread()->terminate();
        m_impl->thread()->quit();
        qDebug() << "terminate returned";
        m_impl->thread()->wait();
        qDebug() << "wait returned";

        qApp->quit();
    }

private:
    ThreadImpl* m_impl;
};

int main( int argc, char** argv ) {
    QApplication app( argc, argv );

    qDebug() << "main thread:" << app.thread();
    KDThreadRunner<ThreadImpl> thread;
    // First solution
    //thread.start();
    // Second solution
    ThreadImpl* impl = thread.startThread();
    qDebug() << "impl=" << impl;

    TestButton* button = new TestButton(impl);
    button->show();

    return app.exec();
}

#include "kdthreadrunner_test.moc"
