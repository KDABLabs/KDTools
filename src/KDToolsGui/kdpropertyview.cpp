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

#include "kdpropertyview.h"
#include "kdpropertymodel.h"
#include "kdpropertyviewdelegate_p.h"

#include <QDebug>
#include <QPainter>
#include <QTimer>

/*!
  \defgroup KDPropertyView KDPropertyView
  \ingroup gui
*/

class KDPropertyView::Private {};

/*!\class KDPropertyView KDPropertyView
  \ingroup KDPropertyView
  \brief A widget for displaying and editing a collection
  of (name,value) properties.

  This widget follows the Qt model/view design,
  but most often it will probably be
  used with the convenient KDPropertyModel model class instead
  of general QAbstractItemModels.

  \image HTML kdpropertyview.png
*/

/*! Constructor. Creates a KDPropertyView with parent \a parent.
 */
KDPropertyView::KDPropertyView( QWidget* parent )
  : QTreeView(parent), d()
{
  setItemDelegate( new KDPropertyViewDelegate(this) );
  setEditTriggers( QAbstractItemView::AllEditTriggers );
  setSelectionBehavior( QAbstractItemView::SelectRows );
  setSelectionMode( QAbstractItemView::ExtendedSelection );
  setTextElideMode(Qt::ElideRight);
  //header()->setResizeMode( QHeaderView::Stretch );
}

KDPropertyView::~KDPropertyView()
{
}

void KDPropertyView::setModel( QAbstractItemModel* model )
{
  QTreeView::setModel(model);
}

/*! Reimplementation from QTreeView. This expands the first level
  of categories.

  \todo: Figure out why it doesn't work on 2nd. reset
*/
void KDPropertyView::reset()
{
  //qDebug() << "KDPropertyView::reset()";
  QTreeView::reset();
  /* By default we want all categories at the top level to
     be expanded */

  /* Due to a bug in Qt-4.1, we need to do this
     delayed.
     TODO: Review when 4.1.1 is out.
  */
  QTimer::singleShot( 0, this, SLOT(expand()) );
}

/*! Expands the top level categories */
void KDPropertyView::expand()
{
    const QAbstractItemModel * const m = model();
    if ( !m )
        return;
    const QModelIndex root = rootIndex();
    for ( int i = 0, end = m->rowCount( root ); i < end ; ++i ) {
        const QModelIndex idx = m->index( i, 0, root );
        if ( m->hasChildren( idx ) )
            setExpanded( idx, true );
    }
}

void KDPropertyView::drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const
{
    QStyleOptionViewItem opt = viewOptions();
    QStyleOptionViewItem option = opt;
    const QAbstractItemModel * const m = model();

    if ( index.column() == 0 && m && m->data( index, KDPropertyModel::ChangedRole ) == true )
      option.font.setBold(true);

    /*
    if ( m && m->data(index,KDPropertyModel::SeparatorRole) == true ) {
        painter->fillRect(rect, option.palette.dark());
    } else if (selectionModel()->isSelected(index)) {
        painter->fillRect(rect, option.palette.brush(QPalette::Highlight));
    }
    */

    if( m && m->parent(index) == QModelIndex() ) {
      painter->fillRect(rect, option.palette.color(QPalette::AlternateBase));
    } else {
      QRect topr = QRect( rect.topLeft(), QPoint( rect.left()+indentation(), rect.bottom() ) );
      painter->fillRect( topr, option.palette.color(QPalette::AlternateBase));
      //painter->drawLine( rect.x()+indentation(), rect.top(), rect.x()+indentation(), rect.bottom());
      painter->fillRect( QRect( topr.topRight(), rect.bottomRight() ), option.palette.base());

      painter->setPen( option.palette.color(QPalette::AlternateBase) );
      painter->drawLine( topr.right(), rect.bottom(), rect.right(), rect.bottom());
    }

    if (m && m->hasChildren(index)) {
        static const int size = 9;
        opt.state |= QStyle::State_Children;
        opt.rect.setRect(rect.width() - (indentation() + size) / 2,
                         rect.y() + (rect.height() - size) / 2, size, size);
	if (isExpanded(index))
	  opt.state |= QStyle::State_Open;
	QColor bg;
	QVariant bgv = m->data( index,
				      Qt::BackgroundColorRole);
	if( qVariantCanConvert<QColor>(bgv) ) {
	  bg = qVariantValue<QColor>(bgv);
	} else {
	  bg = option.palette.color( QPalette::Background );
	}
        painter->fillRect(opt.rect, bg);
        style()->drawPrimitive(QStyle::PE_IndicatorBranch, &opt, painter, this);
    }
}

void KDPropertyView::rowsInserted(const QModelIndex& parent, int start, int end)
{
  qDebug() << "KDPropertyView::rowsInserted()";
  if ( const QAbstractItemModel * const m = model() )
      for( int i = start; i <= end; ++i ) {
          const QModelIndex idx = m->index( i, 0, parent );
          setExpanded( idx, true );
      }
  QTreeView::rowsInserted(parent,start,end);
}

#include "moc_kdpropertyview.cpp"

#ifdef KDTOOLSGUI_UNITTESTS

#include <KDUnitTest/Test>
#include <QLayout>
#include <QDialog>

KDAB_UNITTEST_SIMPLE( KDPropertyView, "kdtools/gui" ) {

    // reproduce KDTO-196:
    QDialog dialog;
    QVBoxLayout vbox( &dialog );
    KDPropertyView view( &dialog );
    vbox.addWidget( &view );
    view.setModel( 0 );
    QTimer::singleShot( 500, &dialog, SLOT(close()) );
    dialog.exec();
}

#endif // KDTOOLSGUI_UNITTESTS
