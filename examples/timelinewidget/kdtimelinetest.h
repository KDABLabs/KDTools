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

#ifndef KDTIMELINETEST_H
#define KDTIMELINETEST_H

#include <KDToolsCore/kdtoolsglobal>

#include <QWidget>

QT_BEGIN_NAMESPACE
class QAction;
class QStackedWidget;
class QDateTimeEdit;
class QToolButton;
class QDateTime;
class QLabel;
QT_END_NAMESPACE

class KDTimeLineWidget;
class KDTimeScrollBar;
class KDTimeLineArea;

class KDTimeLineTest : public QWidget {
    Q_OBJECT
public:
    explicit KDTimeLineTest( QWidget* parent = 0 );

    QDateTime dateTime() const;

    void setTimeLineWidget( KDTimeLineWidget * tl );
    KDTimeLineWidget * timeLineWidget() const;

public Q_SLOTS:
    void setDateTime( const QDateTime & );
Q_SIGNALS:
    void dateTimeChanged( const QDateTime & );

public:
    bool eventFilter( QObject *, QEvent * );
private Q_SLOTS:
    void slotDateTimeEditChanged( const QDateTime & );
    void slotAddWhiteEventMarker();
    void slotAddRedEventMarker();

private:
    QStackedWidget * m_timestack;
    QLabel * m_timelabel;
    QDateTimeEdit * m_timeedit;
    QToolButton * m_snapbutton;
    KDTimeLineArea * m_timelinearea;

    QAction * m_addwhiteevent;
    QAction * m_addredevent;
};

#endif /* KDTIMELINETEST_H */
