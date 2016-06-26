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

#ifndef __KDUNITTEST_STATIC_EXPORT_H__
#define __KDUNITTEST_STATIC_EXPORT_H__

#ifdef UNITTESTS_IN_STATIC_LIB
  #define UNITTESTS_EXPORT
  #define UNITTESTS_IMPORT
#else
  #define UNITTESTS_EXPORT Q_DECL_EXPORT
  #define UNITTESTS_IMPORT Q_DECL_IMPORT
#endif

// These two macros can be used to force the loading of static objects in static libraries.
// Under normal circumstances, the linker automatically discards unused library symbols from the final executable,
// which is a problem for static objects with a constructor performing a specific action such as registration.
// Note that the export macro only needs to be included once per .cpp file (even if there are many static objects in the file).
// Parameters :
// - ID : an file-wide identifier, e.g. the filename without the extension.
//        It shouldn't be quoted, no spaces, and contain only alphanumerical characters.
#define KDAB_EXPORT_STATIC_SYMBOLS( ID ) UNITTESTS_EXPORT extern int _kdab_init_##ID##_static_symbols(){ return 0; }
#define KDAB_IMPORT_STATIC_SYMBOLS( ID ) UNITTESTS_IMPORT extern int _kdab_init_##ID##_static_symbols(); \
    static int fake_init##ID = _kdab_init_##ID##_static_symbols(); \
    static inline void fake_init_suppress_warning_##ID() { (void)fake_init##ID; }

#endif /* __KDUNITTEST_STATIC_EXPORT_H__ */
