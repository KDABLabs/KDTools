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

#ifndef __KDTOOLS_GUI_KDCOLORBUTTON_H__
#define __KDTOOLS_GUI_KDCOLORBUTTON_H__

#include <KDToolsCore/kdtoolsglobal.h>
#include <KDToolsCore/pimpl_ptr.h>

#if QT_VERSION < 0x050000
#include <QtGui/QWidget>
#else
#include <QtWidgets/QWidget>
#endif

QT_BEGIN_NAMESPACE
class QColor;
QT_END_NAMESPACE

class KDTOOLSGUI_EXPORT KDColorButton : public QWidget {
    Q_OBJECT
    Q_PROPERTY( bool allowTransparentColors READ allowTransparentColors WRITE setAllowTransparentColors )
    Q_PROPERTY( bool allowInvalidColors READ allowInvalidColors WRITE setAllowInvalidColors )
    Q_PROPERTY( QColor color READ color WRITE setColor RESET resetColorProperty NOTIFY colorChanged USER true )
    Q_PRIVATE_SLOT( d, void _kd_enableDisableActions() )
    Q_CLASSINFO( "description", "Color request/display widget" )
public:
    explicit KDColorButton( QWidget * parent=0, Qt::WFlags f=0 );
    explicit KDColorButton( const QColor & color, QWidget * parent=0, Qt::WFlags f=0 );
    ~KDColorButton();

    static QPixmap defaultBackgroundPixmap();
    static QColor screenPixel( const QPoint & p );

    bool allowTransparentColors() const;
    bool allowInvalidColors() const;
    QColor color() const;

public Q_SLOTS:
    void setAllowTransparentColors( bool on );
    void setAllowInvalidColors( bool on );
    void setColor( const QColor & color );
    void requestColor();
    void unsetColor();
    void pickScreenColor();
#ifndef QT_NO_CLIPBOARD
    void copy();
    void paste();
#endif

Q_SIGNALS:
    void colorChanged( const QColor & color );

private:
    virtual QColor getColor()
#ifdef QT_NO_COLORDIALOG
        = 0 // pure virtual iff QT_NO_COLORDIALOG
#endif
        ;

public:
    QSize minimumSizeHint() const KDAB_OVERRIDE;
    QSize sizeHint() const KDAB_OVERRIDE;
    bool event( QEvent * ) KDAB_OVERRIDE;

protected:
    void paintEvent( QPaintEvent * ) KDAB_OVERRIDE;
    void keyPressEvent( QKeyEvent * ) KDAB_OVERRIDE;
    void keyReleaseEvent( QKeyEvent * ) KDAB_OVERRIDE;
    void mousePressEvent( QMouseEvent * ) KDAB_OVERRIDE;
    void mouseMoveEvent( QMouseEvent * ) KDAB_OVERRIDE;
    void mouseReleaseEvent( QMouseEvent * ) KDAB_OVERRIDE;
    void mouseDoubleClickEvent( QMouseEvent * ) KDAB_OVERRIDE;
#ifndef QT_NO_DRAGANDDROP
    void dragEnterEvent( QDragEnterEvent * ) KDAB_OVERRIDE;
    void dropEvent( QDropEvent * ) KDAB_OVERRIDE;
#endif
    void changeEvent( QEvent * ) KDAB_OVERRIDE;

private:
    // this is only needed b/c designer can't reset a QColor property:
    /*! \internal */ void resetColorProperty() { setColor( QColor() ); }

private:
    class Private;
    kdtools::pimpl_ptr<Private> d;
};

#endif /* __KDTOOLS_GUI_KDCOLORBUTTON_H__ */
