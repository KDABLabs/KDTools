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

#ifndef __KDTOOLS__GUI__KDSEARCHLINEEDIT_H__
#define __KDTOOLS__GUI__KDSEARCHLINEEDIT_H__

#include <KDToolsCore/pimpl_ptr.h>

#if QT_VERSION < 0x050000
#include <QtGui/QLineEdit>
#else
#include <QtWidgets/QLineEdit>
#endif

class KDTOOLSGUI_EXPORT KDSearchLineEdit : public QLineEdit
{
    Q_OBJECT
    Q_PROPERTY( QString defaultText READ defaultText WRITE setDefaultText )
    Q_PROPERTY( QMenu* menu READ menu WRITE setMenu )
    Q_CLASSINFO( "description", "A line edit usually used to type in text to search" )
public:
    explicit KDSearchLineEdit( QWidget* parent = 0 );
    explicit KDSearchLineEdit( const QString & contents, QWidget * parent=0 );
    ~KDSearchLineEdit();

    QString defaultText() const;

    QMenu* menu() const;
    void setMenu( QMenu* menu );

    QSize sizeHint() const KDAB_OVERRIDE;
    QSize minimumSizeHint() const KDAB_OVERRIDE;

    bool eventFilter( QObject* watched, QEvent* event ) KDAB_OVERRIDE;

public Q_SLOTS:
    void setDefaultText( const QString& defaultText );
    void setText( const QString& text );

protected:
    void resizeEvent( QResizeEvent* event ) KDAB_OVERRIDE;
    void paintEvent( QPaintEvent* event ) KDAB_OVERRIDE;
    void mousePressEvent( QMouseEvent* event ) KDAB_OVERRIDE;
    void mouseMoveEvent( QMouseEvent* event ) KDAB_OVERRIDE;
    void mouseReleaseEvent( QMouseEvent* event ) KDAB_OVERRIDE;

private:
    Q_PRIVATE_SLOT( d, void _kdt_maybeShowDefaultText() )
    Q_PRIVATE_SLOT( d, void _kdt_userEditedText( const QString& text ) )
    
    class Private;
    kdtools::pimpl_ptr< Private > d;
};

#endif
