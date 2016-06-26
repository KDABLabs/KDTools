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

#ifndef __KDTOOLS__GUI__KDTIMESCROLLBAR_H__
#define __KDTOOLS__GUI__KDTIMESCROLLBAR_H__

#include <KDToolsCore/kdtoolsglobal.h>

#include <KDToolsCore/pimpl_ptr.h>

#if QT_VERSION < 0x050000
#include <QtGui/QWidget>
#else
#include <QtWidgets/QWidget>
#endif

class KDTOOLSGUI_EXPORT KDTimeScrollBar : public QWidget {
    Q_OBJECT
    Q_PROPERTY( int value READ value WRITE setValue )
    Q_PROPERTY( int minimumValue READ minimumValue WRITE setMinimumValue )
    Q_PROPERTY( int maximumValue READ maximumValue WRITE setMaximumValue )
    Q_PROPERTY( int zoomValue READ zoomValue WRITE setZoomValue )
    Q_PROPERTY( int minimumZoom READ minimumZoom )
    Q_PROPERTY( int maximumZoom READ maximumZoom )
public:
    explicit KDTimeScrollBar( QWidget * parent=0 );
    virtual ~KDTimeScrollBar();

    void setMinimumValue( int min );
    int minimumValue() const;

    void setMaximumValue( int max );
    int maximumValue() const;

    void setRange( int, int );

    int value() const;
    int zoomValue() const;

    int minimumZoom() const;
    int maximumZoom() const;

    QSize sizeHint() const KDAB_OVERRIDE;
Q_SIGNALS:
    void valueChanged( int );
    void zoomChanged( int );
public Q_SLOTS:
    void setValue( int value );
    void setZoomValue( int zoom );
protected:
    void paintEvent( QPaintEvent * ) KDAB_OVERRIDE;
    void resizeEvent( QResizeEvent * ) KDAB_OVERRIDE;
    void mousePressEvent( QMouseEvent * ) KDAB_OVERRIDE;
    void mouseReleaseEvent( QMouseEvent * ) KDAB_OVERRIDE;
    void mouseMoveEvent( QMouseEvent * ) KDAB_OVERRIDE;
    void keyPressEvent( QKeyEvent * ) KDAB_OVERRIDE;

private:
    class Private;
    kdtools::pimpl_ptr<Private> d;
};

#endif /* __KDTOOLS__GUI__KDTIMESCROLLBAR_H__ */

