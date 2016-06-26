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

#ifndef __KDTOOLS__GUI__KDHELPBUTTON_H__
#define __KDTOOLS__GUI__KDHELPBUTTON_H__

#include <KDToolsCore/pimpl_ptr.h>

#if QT_VERSION < 0x050000
#include <QtGui/QAbstractButton>
#else
#include <QtWidgets/QAbstractButton>
#endif

QT_BEGIN_NAMESPACE
class QStyleOptionButton;
QT_END_NAMESPACE

class KDTOOLSGUI_EXPORT KDHelpButton : public QAbstractButton
{
    Q_OBJECT
    Q_CLASSINFO( "description", "A Mac-style help button" )
public:
    explicit KDHelpButton( QWidget* parent = 0 );
    ~KDHelpButton();

    QSize sizeHint() const KDAB_OVERRIDE;
    QSize minimumSizeHint() const KDAB_OVERRIDE;

protected:
    void initStyleOption( QStyleOptionButton* option ) const;
    void changeEvent( QEvent* event ) KDAB_OVERRIDE;
    void mousePressEvent( QMouseEvent* event ) KDAB_OVERRIDE;
    void resizeEvent( QResizeEvent* event ) KDAB_OVERRIDE;
    void paintEvent( QPaintEvent* event ) KDAB_OVERRIDE;

private:
    class Private;
    kdtools::pimpl_ptr< Private > d;
};

#endif
