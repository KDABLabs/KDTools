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

#ifndef __KDTOOLS_GUI_KDPROPERTYVIEWDELEGATE_P_H__
#define __KDTOOLS_GUI_KDPROPERTYVIEWDELEGATE_P_H__

#include <KDToolsCore/kdtoolsglobal.h>

#if QT_VERSION < 0x050000
#include <QtGui/QItemDelegate>
#else
#include <QtWidgets/QItemDelegate>
#endif

class KDPropertyViewDelegate : public QItemDelegate {
  Q_OBJECT
public:
  explicit KDPropertyViewDelegate( QObject* parent = 0 );

  virtual void paint(QPainter* painter, const QStyleOptionViewItem& option,
		     const QModelIndex& index) const;

  virtual QSize sizeHint(const QStyleOptionViewItem& option,
			 const QModelIndex& index) const;

  virtual QWidget *createEditor(QWidget* parent,
				const QStyleOptionViewItem& option,
				const QModelIndex& index) const;

  virtual void setEditorData(QWidget* editor,
			     const QModelIndex& index) const;

  virtual void setModelData(QWidget* editor,
			    QAbstractItemModel* model,
			    const QModelIndex& index) const;

  virtual void updateEditorGeometry(QWidget* editor,
				    const QStyleOptionViewItem& option,
				    const QModelIndex& index) const;

  virtual bool eventFilter( QObject* watched, QEvent* event );
private Q_SLOTS:
  void sync();
};

#endif /* __KDTOOLS_GUI_KDPROPERTYVIEWDELEGATE_P_H__ */

