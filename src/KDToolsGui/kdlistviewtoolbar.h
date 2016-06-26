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

#ifndef __KDTOOLS__GUI__KDLISTVIEWTOOLBAR_H__
#define __KDTOOLS__GUI__KDLISTVIEWTOOLBAR_H__

#include <KDToolsCore/pimpl_ptr>

#if QT_VERSION < 0x050000
#include <QtGui/QWidget>
#else
#include <QtWidgets/QWidget>
#endif

class KDTOOLSGUI_EXPORT KDListViewToolBar : public QWidget
{
    Q_OBJECT
    Q_CLASSINFO( "description", "Mac-style toolbar to be placed below list views" )
public:
    explicit KDListViewToolBar( QWidget * parent=0 );
    ~KDListViewToolBar();

    using QWidget::addAction;
    QAction* addAction( const QString& text, const QObject* receiver, const char* member );
    QAction* addAction( const QString& text );
    QAction* addSeparator();

    QAction* actionAt( const QPoint& p ) const;
    QAction* actionAt( int x, int y );

    QAction* insertSeparator( QAction* before );

    bool event( QEvent* event ) KDAB_OVERRIDE;
    QSize sizeHint() const KDAB_OVERRIDE;

Q_SIGNALS:
    void actionTriggered( QAction* action );

private:
    Q_PRIVATE_SLOT( d, void _kdt_layoutButtons() )
    Q_PRIVATE_SLOT( d, void _kdt_triggered() )
    
    class Private;
    kdtools::pimpl_ptr< Private > d;
};

#endif
