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

#include "macwidget_p.h"

#include <QString>
#include <QFile>
#include <QStyle>
#ifdef Q_OS_MAC
# include <QMacStyle>
#endif

static void init_resource() {
    Q_INIT_RESOURCE( KDAB_kdtools_macwidgets_resources );
}

QString _macwidget::RESOURCE( const char* name )
{
    const QString result = QString::fromLatin1( ":/.__kdab__/kdtools/macwidgets/%1" ).arg( QLatin1String( name ) );
    if( !QFile::exists( result ) )
        init_resource();
    return result;
}

#ifdef Q_OS_MAC /* otherwise, defined inline in macwidget_p.h */
bool _macwidget::isMacStyle( const QStyle* style )
{
    return qobject_cast< const QMacStyle* >( style )
        || qstrcmp( style->metaObject()->className(), "QStyleSheetStyle" ) == 0;
}
#endif

