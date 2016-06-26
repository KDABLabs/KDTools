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

#include "kdshortcutsplitterhandle_p.h"

#include <QApplication>
#include <QMouseEvent>
#include <QStyleOption>
#include <QStylePainter>

KDShortcutSplitterHandle::KDShortcutSplitterHandle( QWidget* parent )
  : QWidget(parent), m_stepsize(1), m_incrementfrombottom(false), m_hover(false)
{
#ifndef QT_NO_CURSOR
  setCursor( Qt::SplitVCursor );
#endif
  setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );
}

void KDShortcutSplitterHandle::setStepSize(int s)
{
  m_stepsize = s;
}

void KDShortcutSplitterHandle::setIncrementFromBottom(bool b)
{
  m_incrementfrombottom = b;
}

void KDShortcutSplitterHandle::enterEvent(QEvent*)
{
  m_hover = true;
  update();
}

void KDShortcutSplitterHandle::leaveEvent(QEvent*)
{
  m_hover = false;
  update();
}


void KDShortcutSplitterHandle::mousePressEvent(QMouseEvent *e)
{
  //qDebug() << "KDShortcutSplitterHandle::mousePressEvent( " << e << "), pos="<<e->pos();
  if (e->button() == Qt::LeftButton) {
    m_hover = true;
  }
}

void KDShortcutSplitterHandle::mouseMoveEvent( QMouseEvent* e )
{
  //qDebug() << "KDShortcutSplitterHandle::mouseMoveEvent( " << e << "), pos="<<e->pos();
  if (!(e->buttons() & Qt::LeftButton))
    return;
  if( e->globalPos().y()-mapToGlobal(rect().bottomLeft()).y() > stepSize() ) {
    if( incrementFromBottom() ) emit splitterMovedDown();
    else emit splitterMovedUp();
  } else if( mapToGlobal(rect().topLeft()).y()-e->globalPos().y() > stepSize() ) {
    if( incrementFromBottom() ) emit splitterMovedUp();
    else emit splitterMovedDown();
  }
}

QSize KDShortcutSplitterHandle::sizeHint() const
{
  int hw = 5; /* TODO, make configureable? */
  QStyleOption opt(0);
  opt.init(this);
  opt.state = QStyle::State_None;
  return style()->sizeFromContents(QStyle::CT_Splitter, &opt, QSize(hw, hw), this)
    .expandedTo(QApplication::globalStrut());
}

void KDShortcutSplitterHandle::paintEvent( QPaintEvent* ev )
{
  Q_UNUSED(ev);
  QStylePainter p(this);
  QStyleOption opt;
  opt.init(this);
  //opt.state |= QStyle::State_Vertical;
  if(isEnabled())
    opt.state |= QStyle::State_Enabled;
  if(m_hover) opt.state |= QStyle::State_MouseOver;
  p.drawControl( QStyle::CE_Splitter, opt );
}

#include "moc_kdshortcutsplitterhandle_p.cpp"
