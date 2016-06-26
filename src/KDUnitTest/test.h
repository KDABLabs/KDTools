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
#ifndef __KDAB__UNITTEST__TEST_H__
#define __KDAB__UNITTEST__TEST_H__

#ifndef KDAB_NO_UNIT_TESTS

#include "kdunittestglobal.h"

#include <string>
#include <iostream>

namespace KDUnitTest {

    class TestRegistry;

#ifndef DOXYGEN_RUN
#define assertNotNull( x ) this->_assertNotNull( ( x ), #x, __FILE__, __LINE__ )
#define assertNull( x ) this->_assertNull( ( x ), #x, __FILE__, __LINE__ )
#define assertTrue( x )  this->_assertTrue( (x), #x, __FILE__, __LINE__ )
#define assertFalse( x ) this->_assertFalse( (x), #x, __FILE__, __LINE__ )
#define assertEqual( x, y ) this->_assertEqual( (x), (y), #x, #y, __FILE__, __LINE__ )
#define assertNotEqual( x, y ) this->_assertNotEqual( (x), (y), #x, #y, __FILE__, __LINE__ )
#define assertLess( x, y ) this->_assertLess( (x), (y), #x, #y, __FILE__, __LINE__ )
#define assertLessOrEqual( x, y ) this->_assertLessOrEqual( (x), (y), #x, #y, __FILE__, __LINE__ )
#define assertGreater( x, y ) this->_assertGreater( (x), (y), #x, #y, __FILE__, __LINE__ )
#define assertGreaterOrEqual( x, y ) this->_assertGreaterOrEqual( (x), (y), #x, #y, __FILE__, __LINE__ )
#define assertIsNaN( x ) this->_assertIsNaN( (x), #x, __FILE__, __LINE__ )
#define assertIsNotNaN( x ) this->_assertIsNotNaN( (x), #x, __FILE__, __LINE__ )

#define assertThrowsExceptionWithCode( x, E, code ) \
    do {                                            \
        try {                                       \
            x;                                      \
            this->fail( __FILE__, __LINE__ )                      \
                << "\"" #x "\" didn't throw \"" #E "\"" << std::endl;   \
        } catch ( const E & _kdunittest_thrown ) {                      \
            this->success( __FILE__, __LINE__ );                              \
            ( void )_kdunittest_thrown;                                 \
            code;                                                       \
        } catch ( ... ) {                                               \
            this->fail( __FILE__, __LINE__ )                                  \
                << "\"" #x "\" threw something, but it wasn't \"" #E "\"" << std::endl; \
        }                                                               \
    } while ( false )

#define assertThrowsException( x, E ) assertThrowsExceptionWithCode( x, E, do{}while(0) )

#define assertDoesNotThrowException( x, E )                             \
    do {                                                                \
        try {                                                           \
            x;                                                          \
            this->success( __FILE__, __LINE__ );                              \
        } catch ( const E & ) {                                         \
            this->fail( __FILE__, __LINE__ )                                  \
                << "\"" #x "\" threw \"" #E "\", but shouldn't" << std::endl; \
        } catch ( ... ) {                                               \
            this->success( __FILE__, __LINE__ );                                  \
        }                                                               \
    } while ( false )
#endif


  class KDTOOLS_UNITTEST_EXPORT Test {
    friend class ::KDUnitTest::TestRegistry;
    const std::string mName;
    unsigned int mFailed, mSucceeded;
  public:
    Test( const std::string & name );
    virtual ~Test() {}

    const std::string & name() const { return mName; }
    unsigned int failed() const { return mFailed; }
    unsigned int succeeded() const { return mSucceeded; }

    virtual void run() = 0;

  protected:
#ifdef DOXYGEN_RUN
    void assertNotNull( x );
    void assertNull( x );
    void assertTrue( x );
    void assertFalse( x );
    void assertEqual( x, y );
    void assertNotEqual( x, y );
    void assertLess( x, y );
    void assertLessOrEqual( x, y );
    void assertGreater( x, y );
    void assertGreaterOrEqual( x, y );
    void assertIsNaN( x );
    void assertIsNotNaN( x );
    void assertThrowsException( x, E );
    void assertDoesNotThrowException( x, E );
    void assertThrowsExceptionWithCode( x, E, code );
#else
    void _assertNotNull( const void * x, const char * expression, const char * file, unsigned int line );
    void _assertNull( const void * x, const char * expression, const char * file, unsigned int line );
    void _assertIsNaN( double v, const char * expression, const char * file, unsigned int line );
    void _assertIsNotNaN( double v, const char * expression, const char * file, unsigned int line );
    void _assertTrue( bool x, const char * expression, const char * file, unsigned int line );
    void _assertFalse( bool x, const char * expression, const char * file, unsigned int line );

    template <typename T, typename S>
    void _assertEqual( const T & x1, const S & x2, const char * expr1, const char * expr2, const char * file, unsigned int line ) {
      if ( x1 == x2 ) this->success( file, line );
      else this->fail( file, line ) << '"' << expr1 << "\" yielded " << x1 << "; expected: " << x2 << "(\"" << expr2 << "\")" << std::endl;
    }
    template <typename T, typename S>
    void _assertNotEqual( const T & x1, const S & x2, const char * expr1, const char * expr2, const char * file, unsigned int line ) {
      if ( x1 != x2 ) this->success( file, line );
      else this->fail( file, line ) << '"' << expr1 << "\" yielded " << x1 << "; expected something not equal to: " << x2 << "(\"" << expr2 << "\")" << std::endl;
    }
    template <typename T, typename S>
    void _assertLess( const T & x1, const S & x2, const char * expr1, const char * expr2, const char * file, unsigned int line ) {
        if ( x1 < x2 ) this->success( file, line );
        else this->fail( file, line ) << '"' << expr1 << "\" yielded " << x1 << "; expected something less than: " << x2 << "(\"" << expr2 << "\")" << std::endl;
    }
    template <typename T, typename S>
    void _assertLessOrEqual( const T & x1, const S & x2, const char * expr1, const char * expr2, const char * file, unsigned int line ) {
        if ( x1 <= x2 ) this->success( file, line );
        else this->fail( file, line ) << '"' << expr1 << "\" yielded " << x1 << "; expected something less than or equal to: " << x2 << "(\"" << expr2 << "\")" << std::endl;
    }
    template <typename T, typename S>
    void _assertGreater( const T & x1, const S & x2, const char * expr1, const char * expr2, const char * file, unsigned int line ) {
        if ( x1 > x2 ) this->success( file, line );
        else this->fail( file, line ) << '"' << expr1 << "\" yielded " << x1 << "; expected something greater than: " << x2 << "(\"" << expr2 << "\")" << std::endl;
    }
    template <typename T, typename S>
    void _assertGreaterOrEqual( const T & x1, const S & x2, const char * expr1, const char * expr2, const char * file, unsigned int line ) {
        if ( x1 >= x2 ) this->success( file, line );
        else this->fail( file, line ) << '"' << expr1 << "\" yielded " << x1 << "; expected something greater than or equal to: " << x2 << "(\"" << expr2 << "\")" << std::endl;
    }
#endif

  protected:
      void expectNextCheckToFail();
      std::ostream & fail( const char * file, unsigned int line );
      void success( const char * file, unsigned int line );

  private:
      bool checkExpectedToFail() const;
      void expectNextCheckToSucceed();
  };

  class KDTOOLS_UNITTEST_EXPORT TestFactory {
  public:
    virtual ~TestFactory() {}
    virtual Test * create() const = 0;
  };

} // namespace KDUnitTest

#include "testregistry.h"

namespace KDUnitTest {

  template <typename T_Test>
  class MAKEINCLUDES_EXPORT GenericFactory : public TestFactory {
  public:
    explicit GenericFactory( const char * group=0 ) {
      TestRegistry::instance()->registerTestFactory( this, group );
    }
    /* reimp */ T_Test * create() const { return new T_Test(); }
  };

} // namespace KDUnitTest

#include "kdunittest_static_export.h"

#ifdef DOXYGEN_RUN
#define KDAB_NO_UNIT_TESTS
#undef KDAB_NO_UNIT_TESTS
#endif // DOXYGEN_RUN

#define KDAB_EXPORT_UNITTEST( Class, Group ) \
    static const KDUnitTest::GenericFactory< Class > __##Class##_unittest( Group ); \
    KDAB_EXPORT_STATIC_SYMBOLS( Class )

#define KDAB_EXPORT_SCOPED_UNITTEST( Namespace, Class, Group ) \
    static const KDUnitTest::GenericFactory< Namespace::Class > __##Class##_unittest( Group ); \
    KDAB_EXPORT_STATIC_SYMBOLS( Class )

#define KDAB_IMPORT_UNITTEST( Class ) KDAB_IMPORT_STATIC_SYMBOLS( Class )
#define KDAB_IMPORT_UNITTEST_SIMPLE( Class ) KDAB_IMPORT_STATIC_SYMBOLS( Class##Test )

#define KDAB_UNITTEST_SIMPLE( Class, Group )                 \
    class Class##Test : public KDUnitTest::Test {  \
    public:                                                 \
        Class##Test() : Test( #Class ) {}                   \
        void run();                                         \
    };                                                      \
    KDAB_EXPORT_UNITTEST( Class##Test, Group )               \
    void Class##Test::run()

#define KDAB_SCOPED_UNITTEST_SIMPLE( Namespace, Class, Group )   \
    namespace Namespace {                                       \
        class Class##Test : public KDUnitTest::Test {  \
        public:                                                 \
            Class##Test() : Test( #Namespace "::" #Class ){}    \
            void run();                                         \
        };                                                      \
    }                                                           \
    KDAB_EXPORT_SCOPED_UNITTEST( Namespace, Class##Test, Group ) \
    void Namespace::Class##Test::run()

#endif // KDAB_NO_UNIT_TESTS

#endif // __KDAB__UNITTEST__TEST_H__
