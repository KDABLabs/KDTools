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

//
//
// This file is not part of the KDTools API, do not use.
// It is subject to change without notice.
// You have been warned.
//
//
#ifndef __KDAB__UNITTEST__TESTREGISTRY_H__
#define __KDAB__UNITTEST__TESTREGISTRY_H__

#ifndef KDAB_NO_UNIT_TESTS

#include "kdunittestglobal.h"

#include <string>
#include <map>
#include <vector>

namespace KDUnitTest {

    class Test;
    class TestFactory;

    class KDTOOLS_UNITTEST_EXPORT TestRegistry {
        friend class ::KDUnitTest::TestFactory;
        static TestRegistry * mSelf;
        TestRegistry();
        ~TestRegistry();
    public:
        static TestRegistry * instance();
        static void deleteInstance();

        void registerTestFactory( const TestFactory * tf, const char * group );

        unsigned int run() const;
        unsigned int run( const char * group ) const;

    private:
        static unsigned int runTest( const TestFactory * tf );

    private:
        std::map< std::string, std::vector<const TestFactory*> > mTests;
    };

    class KDTOOLS_UNITTEST_EXPORT Runner {
    public:
        ~Runner() { TestRegistry::deleteInstance(); }

        unsigned int run( const char * group=0 ) const {
            if ( group && *group )
                return TestRegistry::instance()->run( group );
            return TestRegistry::instance()->run();
        }
    };

} // namespace KDUnitTest

#endif // KDAB_NO_UNIT_TESTS

#endif /* __KDAB__UNITTEST__TESTREGISTRY_H__ */
