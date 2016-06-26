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

#include <KDUnitTest/Runner>
#include <KDUnitTest/Test>

#include <QApplication>

#include <iostream>
#include <cstdlib>

KDAB_IMPORT_UNITTEST_SIMPLE( KDSignalSpy )
KDAB_IMPORT_UNITTEST_SIMPLE( KDAutoPointer )
KDAB_IMPORT_UNITTEST_SIMPLE( pimpl_ptr )
KDAB_IMPORT_UNITTEST_SIMPLE( KDRect )
#if QT_VERSION >= 0x040200
KDAB_IMPORT_UNITTEST_SIMPLE( KDVariantConverter )
#endif
#if QT_VERSION >= 0x040400 && !defined(QT_NO_SHAREDMEMORY)
KDAB_IMPORT_UNITTEST_SIMPLE( KDLockedSharedMemoryPointer )
#endif
KDAB_IMPORT_UNITTEST_SIMPLE( KDEmailValidator )
KDAB_IMPORT_UNITTEST( KDGenericFactoryTest )
KDAB_IMPORT_UNITTEST_SIMPLE( KDSaveFile )
KDAB_IMPORT_UNITTEST_SIMPLE( KDMetaMethodIterator )
KDAB_IMPORT_UNITTEST_SIMPLE( KDThreadRunner )
KDAB_IMPORT_UNITTEST_SIMPLE( KDMatrixMapper )
KDAB_IMPORT_UNITTEST_SIMPLE( KDTransformMapper )

int main( int argc, char * argv[] ) {

    QApplication app( argc, argv );

    KDUnitTest::Runner r;
    unsigned int failed = 0;
    if ( argc == 1 ) {
        failed = r.run();
    } else {
        for ( int i = 1 ; i < argc ; ++i )
            if ( argv[i] && *argv[i] )
                failed += r.run( argv[i] );
            else
                std::cerr << argv[0] << ": skipping empty group name" << std::endl;
    }

    return failed ? EXIT_FAILURE : EXIT_SUCCESS ;
}


