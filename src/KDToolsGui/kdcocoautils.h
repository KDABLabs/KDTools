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

#ifndef __KDTOOLSCORE_KDCOCOAUTILS_H__
#define __KDTOOLSCORE_KDCOCOAUTILS_H__

#include <QString>

#include "kdtoolsglobal.h"

#if defined(Q_OS_MAC) || defined(DOXYGEN_RUN)

#include <Foundation/Foundation.h>

class KDTOOLSCORE_EXPORT KDCocoaUtils KDAB_FINAL_CLASS
{
public:
    static NSString* fromQString( const QString& string );
    static QString toQString( const NSString* string );
};

#endif /* defined(Q_OS_MAC) || defined(DOXYGEN_RUN) */

#endif /* __KDTOOLSCORE_KDCOCOAUTILS_H__ */
