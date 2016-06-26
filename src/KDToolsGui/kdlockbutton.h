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

#ifndef __KDTOOLS__GUI__KDLOCKBUTTON_H__
#define __KDTOOLS__GUI__KDLOCKBUTTON_H__

#include <KDToolsCore/pimpl_ptr>

#if QT_VERSION < 0x050000
#include <QtGui/QAbstractButton>
#else
#include <QtWidgets/QAbstractButton>
#endif

class KDTOOLSGUI_EXPORT KDLockButton : public QAbstractButton
{
    Q_OBJECT
    Q_PROPERTY( QString lockedText READ lockedText WRITE setLockedText )
    Q_PROPERTY( bool locked READ isLocked WRITE setLocked )
    Q_PROPERTY( bool autoLock READ autoLock WRITE setAutoLock )
    Q_CLASSINFO( "description", "A Mac-style widget to indicate GUI elements are locked from/by the user" )
public:
    explicit KDLockButton( QWidget* parent = 0 );
    ~KDLockButton();

    void setLockedText( const QString& lockedText );
    QString lockedText() const;

    bool autoLock() const;
    void setAutoLock( bool autoLock );

    bool isLocked() const;

    /* reimp */ QSize sizeHint() const;
    /* reimp */ QSize minimumSizeHint() const;

public Q_SLOTS:
    void lock();
    void unlock();
    void setLocked( bool locked );

Q_SIGNALS:
    void locked();
    void unlocked();
    void lockChanged( bool locked );
    void lockRequested();
    void unlockRequested();

protected:
    /* reimp */ void nextCheckState();
    /* reimp */ void paintEvent( QPaintEvent* event );

private:
    Q_PRIVATE_SLOT( d, void _kdt_checkChanged( bool checked ) )

    class Private;
    kdtools::pimpl_ptr< Private > d;
};

#endif
