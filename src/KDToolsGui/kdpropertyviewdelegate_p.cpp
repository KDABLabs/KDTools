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

#include "kdpropertyviewdelegate_p.h"
#include "kdproperty.h"
#include "kdpropertyview.h"
#include "kdpropertymodel.h"
#include "kdpropertyeditor.h"

#include <QApplication>
#include <QMouseEvent>
#include <QPainter>

/*!\class KDPropertyViewDelegate KDPropertyViewDelegate
  \ingroup KDPropertyView
  \brief A QItemDelegate subclass used internally by
  KDPropertyView to display and edit properties.

  By default, KDPropertyViewDelegate will use the
  standard Qt editor widgets from the default
  QItemEditorFactory, but if the items in the
  model offer a value for the KDPropertyModel::EditorTypeRole
  role, KDPropertyEditorFactory will be consulted instead
  for an editor class with the name returned from the
  KDPropertyModel::EditorTypeRole value.
*/

/* From qitemdelegate.cpp */
static const int textMargin = 1;

KDPropertyViewDelegate::KDPropertyViewDelegate( QObject* parent )
  : QItemDelegate(parent)
{
}

void  KDPropertyViewDelegate::paint(QPainter* painter, const QStyleOptionViewItem& opt,
				      const QModelIndex& index) const
{
  QStyleOptionViewItem option = opt;

  const QAbstractItemModel *model = index.model();
  QVariant changed = model->data( index, KDPropertyModel::ChangedRole );
  if (index.column() == 0 &&  changed != QVariant() && changed.toBool() == true ) {
    option.font.setBold(true);
  }

  /*
  if (property && property->isSeparator()) {
    option.palette.setColor(QPalette::Text, option.palette.color(QPalette::BrightText));
    option.font.setBold(true);
    option.state &= ~QStyle::State_Selected;
  }
  */

  if (index.column() == 1) {
    option.state &= ~QStyle::State_Selected;
  }

  option.state &= ~QStyle::State_HasFocus;

  /*
  if (property->isSeparator()) {
    QBrush bg = option.palette.dark();
    painter->fillRect(option.rect, bg);
  }
  */


  /* Below we take care of rendering item types that need
     to look like their special editors. A bit unfortunate that
     it is hardcoded here, but ... */
  QVariant editortypev;
  if( index.column() == 1 &&
      (editortypev = model->data(index, KDPropertyModel::EditorTypeRole)) != QVariant() ) {
    QString editortype = editortypev.toString();
    if( editortype == QLatin1String( "KDBooleanPropertyEditor" ) ) {
      Qt::CheckState cs = model->data(index,Qt::EditRole).toBool()?Qt::Checked:Qt::Unchecked;
#if QT_VERSION < 0x050000
      const QRect checkRect = check(option, option.rect,cs);
#else
      const QRect checkRect = doCheck(option, option.rect,cs);
#endif
      QStyleOptionViewItem tmpopt = option;
      tmpopt.rect.adjust(1,0,-1,0);
      tmpopt.rect.moveLeft(0);
      tmpopt.rect.moveTop(0);
      drawCheck(painter,tmpopt,checkRect.translated(tmpopt.rect.topLeft()),cs);
    } else if( editortype == QLatin1String( "KDChoicePropertyEditor" ) ) {
      const QStringList opts = model->data(index,KDPropertyModel::ChoiceOptionsHint).toStringList();
      int value = model->data(index,Qt::EditRole).toInt();
      QStyle* style = QApplication::style();
      QRect totalRect = option.rect.adjusted(2,2,-2,-2);
      int h = totalRect.height()/opts.size();
      QRect oneRect = QRect( totalRect.left(), totalRect.top(), totalRect.width(), h );
      int i = 0;
      for( QStringList::const_iterator it = opts.begin(); it != opts.end(); ++it ) {
          QStyleOptionButton rbopt;
          rbopt.text = *it;
          rbopt.state |= i++ == value ? QStyle::State_On:QStyle::State_Off;
          rbopt.rect = oneRect;
          style->drawControl( QStyle::CE_RadioButton, &rbopt, painter, static_cast<QWidget*>(parent()) );
          oneRect.translate( 0,h );
      }
    } else if( editortype == QLatin1String( "KDMultipleChoicePropertyEditor" ) ) {
      const QStringList opts = model->data(index,KDPropertyModel::ChoiceOptionsHint).toStringList();
      QList<QVariant> values = qVariantValue<QList<QVariant> >(model->data( index, Qt::EditRole ));
      QStyle* style = QApplication::style();
      QRect totalRect = option.rect.adjusted(2,2,-2,-2);
      int h = totalRect.height()/opts.size();
      QRect oneRect = QRect( totalRect.left(), totalRect.top(), totalRect.width(), h );
      int i = 0;
      for( QStringList::const_iterator it = opts.begin(); it != opts.end(); ++it ) {
          QStyleOptionButton cbopt;
          cbopt.text = *it;
          cbopt.state |= values.contains(i++) ? QStyle::State_On:QStyle::State_Off;
          cbopt.rect = oneRect;
          style->drawControl( QStyle::CE_CheckBox, &cbopt, painter, static_cast<QWidget*>(parent()) );
          oneRect.translate( 0,h );
      }
    } else {
      QItemDelegate::paint(painter, option, index);
    }
  } else {
    QItemDelegate::paint(painter, option, index);
  }

  /* grid */
  painter->setPen( option.palette.color( QPalette::AlternateBase ) );
  painter->drawLine(option.rect.right(), option.rect.y(),
		    option.rect.right(), option.rect.bottom());
  painter->drawLine(option.rect.x(), option.rect.bottom(),
		    option.rect.right(), option.rect.bottom());
}


QSize KDPropertyViewDelegate::sizeHint(const QStyleOptionViewItem& option,
					 const QModelIndex& index) const
{
  QSize s = QItemDelegate::sizeHint(option, index) + QSize(4, 4); /* TODO?? */
  if( index.model()->data(index,KDPropertyModel::EditorTypeRole).toString() == QLatin1String( "KDChoicePropertyEditor" ) ) {
    s.rheight() = s.height()*index.model()->data(index,KDPropertyModel::ChoiceOptionsHint).toStringList().size();
  }
  if( index.model()->data(index,KDPropertyModel::EditorTypeRole).toString() == QLatin1String( "KDMultipleChoicePropertyEditor" ) ) {
    s.rheight() = s.height()*index.model()->data(index,KDPropertyModel::ChoiceOptionsHint).toStringList().size();
  }
  return s;
}

QWidget* KDPropertyViewDelegate::createEditor(QWidget* parent,
						const QStyleOptionViewItem& option,
						const QModelIndex& index) const
{
  const QAbstractItemModel *model = index.model();

  QWidget *editor = 0;

  if ( /*!isReadOnly() &&*/ model->flags(index)&Qt::ItemIsEditable) {
    QVariant editortype = model->data( index, KDPropertyModel::EditorTypeRole );
    if( KDPropertyEditor* iface = KDPropertyEditorFactory::defaultFactory()->editor(editortype.toString()) ) {
      editor = iface->createEditor(parent,this,SLOT(sync()));
      //editor->installEventFilter(const_cast<KDPropertyViewDelegate*>(this));
    } else {
      editor = QItemDelegate::createEditor(parent, option, index );
    }
  }
  return editor;
}

void KDPropertyViewDelegate::setEditorData(QWidget* editor,
			     const QModelIndex& index) const
{
  const QAbstractItemModel *model = index.model();
  QVariant editortype = model->data( index, KDPropertyModel::EditorTypeRole );
  if( KDPropertyEditor* iface = KDPropertyEditorFactory::defaultFactory()->editor(editortype.toString()) ) {
    iface->setEditorData(editor,index);
  } else {
    QItemDelegate::setEditorData(editor,index);
  }
}

void KDPropertyViewDelegate::setModelData(QWidget* editor,
			    QAbstractItemModel* model,
			    const QModelIndex& index) const
{
  QVariant editortype = model->data( index, KDPropertyModel::EditorTypeRole );
  //qDebug() << "editortype="<<editortype;
  if( KDPropertyEditor* iface = KDPropertyEditorFactory::defaultFactory()->editor(editortype.toString()) ) {
    iface->setModelData(editor,model,index);
  } else {
    QItemDelegate::setModelData(editor,model,index);
  }
}

void KDPropertyViewDelegate::updateEditorGeometry(QWidget* editor,
				    const QStyleOptionViewItem& option,
				    const QModelIndex& index) const
{
  QItemDelegate::updateEditorGeometry(editor, option, index);
  editor->setGeometry(editor->geometry().adjusted(1, 1, -1, -1));
}

void KDPropertyViewDelegate::sync()
{
  QWidget *w = qobject_cast<QWidget*>(sender());
  if(w == 0) return;
  emit commitData(w);
}

bool KDPropertyViewDelegate::eventFilter( QObject* watched, QEvent* ev )
{
#if QT_VERSION < 0x040200
    /* Bugfix for Qt 4.0 and 4.1.
     * This is basically the relevant part
     * of QItemDelegate::eventFilter(...) copied
     * from Qt 4.2.1
     */
    if ( ev->type() == QEvent::FocusOut ) {
        QWidget *editor = ::qobject_cast<QWidget*>(watched);
        if (!editor->isActiveWindow() || (QApplication::focusWidget() != editor)) {
            // Opening a modal dialog will start a new eventloop
            // that will process the deleteLater event.
            if (QApplication::activeModalWidget() && !QApplication::activeModalWidget()->isAncestorOf(editor))
                return false;
        }
    }
#endif
    return QItemDelegate::eventFilter( watched, ev );
}

#include "moc_kdpropertyviewdelegate_p.cpp"
