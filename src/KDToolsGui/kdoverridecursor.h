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

#ifndef __KDTOOLS__GUI__KDOVERRIDECURSOR_H__
#define __KDTOOLS__GUI__KDOVERRIDECURSOR_H__

#include <KDToolsCore/kdtoolsglobal.h>

#ifndef QT_NO_CURSOR

#include <QtGui/QCursor>

class KDDisableOverrideCursor;

class KDTOOLSGUI_EXPORT KDOverrideCursor KDAB_FINAL_CLASS {
    Q_DISABLE_COPY( KDOverrideCursor )
    friend class KDDisableOverrideCursor;
public:
    KDOverrideCursor();
    explicit KDOverrideCursor( const QCursor & c );
    ~KDOverrideCursor();

    void disable();
    void enable();

private:
    const QCursor cursor;
    bool enabled;
};

class KDTOOLSGUI_EXPORT KDDisableOverrideCursor KDAB_FINAL_CLASS {
    Q_DISABLE_COPY( KDDisableOverrideCursor )
public:
    explicit KDDisableOverrideCursor( KDOverrideCursor & orc );
    ~KDDisableOverrideCursor();
private:
    KDOverrideCursor & overrideCursor;
    bool reserved;
};

#endif // QT_NO_CURSOR

#endif /* __KDTOOLS__GUI__KDOVERRIDECURSOR_H__ */

