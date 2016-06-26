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

#include "kdtimelinetest.h"

#include <KDToolsGui/KDTimeLineWidget>
#include <KDToolsGui/KDTimeScrollBar>

#include <QtGui>

KDTimeLineTest::KDTimeLineTest( QWidget * p )
  : QWidget( p )
{
  m_timestack = new QStackedWidget;
  m_timelabel = new QLabel;
  m_timeedit = new QDateTimeEdit;
  m_snapbutton = new QToolButton;
  m_timelinearea = new KDTimeLineArea;

  m_timelabel->setFocusPolicy( Qt::StrongFocus );
  m_timestack->setSizePolicy( m_timeedit->sizePolicy() );
  m_timestack->addWidget( m_timelabel );
  m_timestack->addWidget( m_timeedit );
  m_snapbutton->setText( tr("Snap" ) );
  m_snapbutton->setCheckable( true );
  m_snapbutton->setChecked( true );
  m_snapbutton->setToolTip( tr( "Snap to grid" ) );


  QToolButton * leftEventTB = new QToolButton;
  QToolButton * rightEventTB = new QToolButton;
  leftEventTB->setArrowType( Qt::LeftArrow );
  leftEventTB->setToolTip( tr("Previous Event") );
  rightEventTB->setArrowType( Qt::RightArrow );
  rightEventTB->setToolTip( tr("Next Event") );
  // from QTabBar
  int arrowwidth = qMax(style()->pixelMetric(QStyle::PM_TabBarScrollButtonWidth, 0, this),
                        QApplication::globalStrut().width());
  leftEventTB->setFixedWidth(arrowwidth);
  rightEventTB->setFixedWidth(arrowwidth);

  setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Fixed );

  m_timelabel->installEventFilter(this);
  m_timeedit->installEventFilter(this);
  m_timelabel->setText( dateTime().toString() );
  m_timeedit->setDateTime( dateTime() );

  m_addwhiteevent = new QAction( tr("Add White Event"), this );
  m_addredevent = new QAction( tr("Add Red Event"), this );
  m_timelinearea->setContextMenuPolicy( Qt::ActionsContextMenu );
  m_timelinearea->addAction(m_addwhiteevent);
  m_timelinearea->addAction(m_addredevent);

  QVBoxLayout* toplayout = new QVBoxLayout( this );
  QVBoxLayout* buttonLayout = new QVBoxLayout;
  QHBoxLayout* eventButtonLayout = new QHBoxLayout;
  QHBoxLayout* timelineLayout = new QHBoxLayout;

  toplayout->addWidget( m_timestack );
  toplayout->addLayout( timelineLayout );

  eventButtonLayout->addWidget( leftEventTB );
  eventButtonLayout->addWidget( rightEventTB );
  buttonLayout->addWidget( m_snapbutton );
  buttonLayout->addLayout( eventButtonLayout );

  timelineLayout->addWidget( m_timelinearea );
  timelineLayout->addLayout( buttonLayout );

  setLayout(toplayout);

  connect( m_addwhiteevent, SIGNAL(triggered()),
           this, SLOT(slotAddWhiteEventMarker()));
  connect( m_addredevent, SIGNAL(triggered()),
           this, SLOT(slotAddRedEventMarker()));
  connect( m_timeedit, SIGNAL(dateTimeChanged(QDateTime)),
           this, SLOT(slotDateTimeEditChanged(QDateTime)) );
  connect( leftEventTB, SIGNAL(clicked()),
           m_timelinearea, SLOT(previousItem()));
  connect( rightEventTB, SIGNAL(clicked()),
           m_timelinearea, SLOT(nextItem()));
  connect( m_snapbutton, SIGNAL(toggled(bool)),
           m_timelinearea->timeLineWidget(), SLOT(setSnapToTickmarks(bool)));
  connect( timeLineWidget(), SIGNAL(elapsedDateTimeChanged(QDateTime)),
           this, SLOT(setDateTime(QDateTime)) );
  connect( timeLineWidget(), SIGNAL(currentDateTimeChanged(QDateTime)),
               m_timeedit, SLOT(setDateTime(QDateTime)) );
}

void KDTimeLineTest::setTimeLineWidget( KDTimeLineWidget* tl )
{
  m_timelinearea->setTimeLineWidget(tl);
  connect( tl, SIGNAL(currentDateTimeChanged(QDateTime)),
           m_timeedit, SLOT(setDateTime(QDateTime)));
}

KDTimeLineWidget * KDTimeLineTest::timeLineWidget() const {
    return m_timelinearea->timeLineWidget();
}

bool KDTimeLineTest::eventFilter( QObject* ob, QEvent* ev )
{
  if( ob == m_timelabel && ev->type() == QEvent::FocusIn ) {
    m_timestack->setCurrentWidget( m_timeedit );
  } else if( ob == m_timeedit && ev->type() == QEvent::FocusOut ) {
    m_timestack->setCurrentWidget( m_timelabel );
  }
  return false;
}

void KDTimeLineTest::setDateTime( const QDateTime& dt )
{
  m_timeedit->setDateTime(dt);
  m_timelabel->setText(dt.toString( m_timeedit->displayFormat() ));
  m_timelinearea->setElapsedDateTime(dt);
}

QDateTime KDTimeLineTest::dateTime() const {
    return m_timelinearea->currentDateTime();
}

void KDTimeLineTest::slotDateTimeEditChanged(const QDateTime& dt)
{
  m_timelinearea->setCurrentDateTime(dt);
  m_timelabel->setText( dateTime().toString() );
}

void KDTimeLineTest::slotAddWhiteEventMarker()
{
  KDTimeLineWidgetItem* item = new KDTimeLineWidgetItem( m_timelinearea->timeLineWidget()->currentDateTime() );
  QPalette pal = item->palette();
  pal.setColor( QPalette::Inactive, QPalette::Background, Qt::white );
  pal.setColor( QPalette::Active, QPalette::Background, Qt::black );
  item->setPalette(pal);
  m_timelinearea->timeLineWidget()->addItem(item);
}

void KDTimeLineTest::slotAddRedEventMarker()
{
  KDTimeLineWidgetItem* item = new KDTimeLineWidgetItem( m_timelinearea->timeLineWidget()->currentDateTime() );
  QPalette pal = item->palette();
  pal.setColor( QPalette::Inactive, QPalette::Background, Qt::red );
  pal.setColor( QPalette::Active, QPalette::Background, Qt::black );
  item->setPalette(pal);
  m_timelinearea->timeLineWidget()->addItem(item);
}

#include "moc_kdtimelinetest.cpp"
