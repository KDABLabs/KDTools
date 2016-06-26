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

#ifndef KDAB_NO_UNIT_TESTS

#include "test.h"

#ifdef TEMPORARILY_REMOVED
#include <libkdabfakes/fakes.h>
#endif

#include <cmath>
#ifdef Q_OS_MAC
using std::isnan;
#endif

#include <limits>
#include <climits>

#if defined (Q_OS_WIN) && defined(_MSC_VER)
#define isnan(x) _isnan(x)
#endif

// MSB, and only MSB, set:
static const unsigned int XFAIL_MARKER = ( UINT_MAX / 2 ) ^ UINT_MAX ;

/*!
  \defgroup unittest KDUnitTest
  \since_l 2.1

  KDUnitTest is a small unit test framework that is designed to be as
  simple as possible, but not simpler. KDUnitTest works best for
  unit-testing of non-GUI classes, although it is extensible enough to
  meet most needs. Its only dependencies are the standard C++
  library. Qt is needed only for some macros, there are no runtime
  dependencies on Qt.

  KDUnitTest currently supports only textual output, but XML as well
  as custom output formats are planned for a future release.

  \sect Terminology

  A KDUnitTest run consists of executing \em tests in all or a subset
  of \em groups. Groups are simple path-like strings, such as
  "kdtools", "kdtools/core", etc. If you run all tests in "kdtools",
  that would include the tests in "kdtools/core", too. One Test may
  contain an arbitrary number of \em checks. A check is the smallest
  unit that can be reported on. A test, on the other hand, is the
  smallest unit of \em execution.

  \sect Simple Usage

  In the simplest case, when your setup and teardown code is trivial,
  usage is as simple as adding

  \code
  #ifndef KDAB_NO_UNIT_TESTS
  #include <KDUnitTest/Test>
  KDAB_UNITTEST_SIMPLE( QString, "Qt/Core/Tools" ) {
     const QString null, empty(""), foo("Foo");
     assertTrue( null.isNull() );
     assertTrue( null.isEmpty() );
     assertFalse( empty.isNull() );
     assertTrue( empty.isEmpty() );
     assertEqual( foo.length(), 3 );
  }
  #endif / KDAB_NO_UNIT_TESTS
  \endcode

  to your implementation file.

  A failed check (assert*()) does not cause the following checks to be
  skipped. If you need that behaviour, add a guard:
  \code
  assertFalse( empty.isNull() );
  if ( !empty.isNull() )
      assertEqual( *empty.data(), QChar('\0') );
  \endcode

  To run the tests, instantiate a KDUnitTest::Runner and call \link
  KDUnitTest::Runner::run() run()\endlink on it:
  \code
  KDUnitTest::Runner runner;
  const unsigned int numFailed = runner.run();
  \endcode

  You can add this code to your application's main() function, or as
  part of a dedicated test runner. It will find all the tests by
  itself and execute them all (but see the section on static libraries
  below).

  \sect Extended Usage

  If you need more control over the test runs, you can write your own
  KDUnitTest::Test subclass (instead of using the one that
  KDAB_UNITTEST_SIMPLE() creates for you), and use the constructor for
  setup and the destructor for teardown functionality. You would then
  use KDAB_EXPORT_UNITTEST() or KDUnitTest::GenericFactory directly,
  to make the test known to KDUnitTest::TestRegistry:

  \code
  #ifndef KDAB_NO_UNIT_TESTS
  #include <KDUnitTest/Test>
  class MyTest : public KDUnitTest::Test {
      QFile file;
  public:
      MyTest()
          : KDUnitTest::Test( "my test" ),
            file( "testdata.dat" )
      {
          file.open( QIODevice::ReadOnly );
      }
      void run() {
          if ( !file.isOpen() ) {
              fail( __FILE__, __LINE__ )
                  << "Could not open file testdata.dat: "
                  << qPrintable( file.errorString() ) ;
              return;
          }
          // checks go here...
      }
      ~MyTest() {
          file.close();
      }
  };
  KDAB_EXPORT_UNITTEST( MyTest, "my group" );
  #endif // KDAB_NO_UNIT_TESTS
  \endcode

  \sect Unit Tests in Static Libraries

  When using a dedicated test runner application, it is most
  convenient to dynamically link the libraries that contain the test
  code. This way, you just have to add new libraries to the linker
  line, and all tests are automatically discovered by
  KDUnitTest::TestRegistry through the use of static initializers.

  When using static libraries, and esp. when using a dedicated test
  runner application (as opposed to the application you're writing
  itself), chances are that the linker will omit most if not all of
  your translation units from the final executable, as nothing in the
  test runner directly references symbols from those translation
  units.

  In this case, you need to add KDAB_IMPORT_UNITTEST() calls to the
  source code of the test runner application to force the linker to
  include the test code in the test runner executable:

  \code
  #include <KDUnitTest/Runner>
  #include <KDUnitTest/Test>

  KDAB_IMPORT_UNITTEST( Test1 );
         :
  KDAB_IMPORT_UNITTEST( TestN );

  int main( int argc, char * argv[] ) {
      // ... init ...

      KDUnitTest::Runner runner;
      return runner.run( argc == 1 ? 0 : argv[1] );
  }
  \endcode
*/

/*!
  \class KDUnitTest::Test
  \since_c 2.1
  \ingroup unittest
  \brief A container for Tests
*/

/*!
  \fn Test::assertNotNull( x )
  Checks that \a x is not NULL.
  \pre \a x needs to be convertible to const void *
*/

/*!
  \fn Test::assertNull( x )
  Checks that \a x is NULL.
  \pre \a x needs to be convertible to const void *
*/

/*!
  \fn Test::assertTrue( x )
  Checks that \a x is \c true.
  \pre \a x needs to be convertible to \c bool
*/

/*!
  \fn Test::assertFalse( x )
  Checks that \a x is \c false.
  \pre \a x needs to be convertible to \c bool
*/

/*!
  \fn Test::assertEqual( x, y )
  Checks that \a x and \a y are equal.
  \pre \a x == \a y needs to be well-defined, and convertible to \c bool
  \pre std::ostream << x and std::ostream << y need to be well-defined
*/

/*!
  \fn Test::assertNotEqual( x, y )
  Checks that \a x and \a y are \em not equal.
  \pre \a x != \a y needs to be well-defined, and convertible to \c bool
  \pre std::ostream << x and std::ostream << y need to be well-defined
*/

/*!
  \fn Test::assertLess( x, y )
  Checks that \a x is less than \a y.
  \pre \a x < \a y needs to be well-defined, and convertible to \c bool
  \pre std::ostream << x and std::ostream << y need to be well-defined
*/

/*!
  \fn Test::assertLessOrEqual( x, y )
  Checks that \a x is less than or equal to \a y.
  \pre \a x <= \a y needs to be well-defined, and convertible to \c bool
  \pre std::ostream << x and std::ostream << y need to be well-defined
*/

/*!
  \fn Test::assertGreater( x, y )
  Checks that \a x is greater than \a y.
  \pre \a x > \a y needs to be well-defined, and convertible to \c bool
  \pre std::ostream << x and std::ostream << y need to be well-defined
*/

/*!
  \fn Test::assertGreaterOrEqual( x, y )
  Checks that \a x is greater than or equal to \a y.
  \pre \a x >= \a y needs to be well-defined, and convertible to \c bool
  \pre std::ostream << x and std::ostream << y need to be well-defined
*/

/*!
  \fn Test::assertIsNaN( x )
  Checks that \a x is a NaN.
  \pre \a x needs to be convertible to \c double
*/

/*!
  \fn Test::assertIsNotNaN( x )
  Checks that \a x is \em not a NaN.
  \pre \a x needs to be convertible to \c double
*/

/*!
  \fn Test::assertThrowsException( x, E )
  Checks that expression \a x throws exception \a E (or a subclass of \a E).
*/

/*!
  \fn Test::assertDoesNotThrowException( x, E )
  Checks that expression \a x does \em not throw exception \a E (or any subclass of \a E).
*/

/*!
  \fn Test::assertThrowsExceptionWithCode( x, E, code )

  Checks that expression \a x throws exception \a E (or a subclass of \a E).

  If this test succeeds (an \a E is thrown), \a code is executed. \a
  code may refer to \c _kdunittest_thrown, which contains a const
  reference to the instance of \a E that was thrown.

  Example:
  \code
  assertThrowsExceptionWithCode( someFunc(), std::exception,
                                 assertNotNull( _kdunittest_thrown.what() ) );
  \endcode
*/

using namespace KDUnitTest;

/*!
  Constructor. Constructs a Test with name \a n.
*/
Test::Test( const std::string & n )
    : mName( n ), mFailed( 0 ), mSucceeded( 0 ) {}

/*!
  \fn Test::~Test()
  Destructor.
*/

/*!
  \fn Test::name() const

  Returns the name of the test, as passed to
  \link Test::Test(const std::string&) the constructor\endlink.
*/

/*!
  \fn Test::failed() const

  Returns the number of checks that failed (number of calls to
  fail()).
*/

/*!
  \fn Test::succeeded() const

  Returns the number of checks that succeeded (number of calls to
  success()).
*/

void Test::_assertNotNull( const void * x, const char * expression, const char * file, unsigned int line ) {
    if ( x ) success( file, line );
    else fail( file, line ) << '"' << expression << "\" is NULL, expected non-NULL" << std::endl;
}

void Test::_assertNull( const void * x, const char * expression, const char * file, unsigned int line ) {
    if ( !x ) success( file, line );
    else fail( file, line ) << '"' << expression << "\" is not NULL, expected NULL" << std::endl;
}

void Test::_assertIsNaN( double d, const char * expression, const char * file, unsigned int line ) {
    if ( isnan( d ) ) success( file, line );
    else fail( file, line ) << '"' << expression << "\" yielded " << d << "; expected NaN" << std::endl;
}

void Test::_assertIsNotNaN( double d, const char * expression, const char * file, unsigned int line ) {
    if ( !isnan( d ) ) success( file, line );
    else fail( file, line ) << '"' << expression << "\" yielded nan; expected something else" << std::endl;
}

void Test::_assertTrue( bool x, const char * expression, const char * file, unsigned int line ) {
    if ( x ) success( file, line );
    else fail( file, line ) << '"' << expression << "\" != TRUE" << std::endl;
}

void Test::_assertFalse( bool x, const char * expression, const char * file, unsigned int line ) {
    if ( !x ) success( file, line );
    else fail( file, line ) << '"' << expression << "\" != FALSE" << std::endl;
}

/*!
  Mark the next check (assertXYZ) as expected to fail.

  This is useful for checks that trigger a bug: The first commit adds
  the test, as an expected failure, the next commit adds the fix and
  removes the expectNextCheckToFail() line.
*/
void Test::expectNextCheckToFail() {
    mFailed |= XFAIL_MARKER;
}

void Test::expectNextCheckToSucceed() {
    mFailed &= ~XFAIL_MARKER;
}

bool Test::checkExpectedToFail() const {
    return mFailed & XFAIL_MARKER;
}

/*!
  Records one successful check (assertXYZ) at file \a file, line \a line.
*/
void Test::success( const char * file, unsigned int line ) {
    if ( checkExpectedToFail() ) {
        expectNextCheckToSucceed();
        fail( file, line ) << "expected failure, got success" << std::endl;
    } else {
        ++mSucceeded;
    }
}

/*!
  Records one failed check (assertXYZ) at file \a file, line \a line.
  Returns a std::ostream that you can use to give more information.
*/
std::ostream & Test::fail( const char * file, unsigned int line ) {
    if ( checkExpectedToFail() ) {
        expectNextCheckToSucceed();
        success( file, line );
        std::cerr << 'X';
    } else {
        ++mFailed;
    }
    return std::cerr << "FAIL: " << file << ':' << line << ": ";
}

/*!
  \fn Test::run()
  Implement this function to contain your checks
*/



/*!
  \class KDUnitTest::TestFactory
  \ingroup unittest
  \short Factory for \link KDUnitTest::Test KDUnitTest::Tests\endlink

  TestFactory is the interface for test factories. You usually will
  not implement this interface yourself, but use GenericFactory instead.
*/

/*!
  \fn TestFactory::~TestFactory()
  Virtual destructor.
*/

/*!
  \fn TestFactory::create() const
  Creates and returns a new instance of a KDUnitTest::Test.
*/


/*!
  \class KDUnitTest::GenericFactory
  \ingroup unittest
  \short Generic factory implementation

  GenericFactory can be used to automatically register
  KDUnitTest::Test classes with TestRegistry:
  \code
  class MyTest : public KDUnitTest::Test {
      // ...
  };
  static const KDUnitTest::GenericFactory<MyTest> fact( "mygroup" );
  \endcode

  However, this is what KDAB_EXPORT_UNITTEST() does, so you will
  usually not use this class directly.
*/

/*!
  \fn GenericFactory::GenericFactory( const char * group )
  Constructor. Registers this factory with TestRegistry in group \a group.
*/

/*!
  \def KDAB_NO_UNIT_TESTS
  \ingroup unittest
  \hideinitializer

  This macro is used to switch unit test compilation off. It also
  hides all definitions of KDUnitTest from the compiler. Therefore,
  you need to wrap your unit test code in it:
  \code
  #ifndef KDAB_NO_UNIT_TESTS
  #include <KDUnitTest/Test>
  KDAB_UNITTEST_SIMPLE( MyClass, "mygroup" ) {
      // ...
  }
  #endif // KDAB_NO_UNIT_TESTS
  \endcode
*/

/*!
  \def KDAB_EXPORT_UNITTEST( Class, Group )
  \ingroup unittest
  \hideinitializer

  Use this macro to register your \link KDUnitTest::Test Test\endlink
  subclass \a Class with \link KDUnitTest::TestRegistry TestRegistry\endlink
  in group \a Group so that it gets executed by the test runner.

  Example:
  \code
  class MyTest : public KDUnitTest::Test {
      // ...
  };
  KDAB_EXPORT_UNITTEST( MyTest, "mygroup" )
  \endcode

  This is all you need to do if your test resides in a dynamic library
  (.so, .dll, .dynlib). If, however, your test resides in a \em static
  library instead (.a, .lib), use KDAB_IMPORT_UNITTEST() with the same
  \a Class argument in the testrunner source.

  \sa KDAB_IMPORT_UNITTEST()
*/

/*!
  \def KDAB_EXPORT_SCOPED_UNITTEST( Namespace, Class, Group )
  \ingroup unittest
  \hideinitializer

  This is a variant of KDAB_EXPORT_UNITTEST() for \link KDUnitTest::Test
  Test\endlink classes in namespaces.
*/

/*!
  \def KDAB_IMPORT_UNITTEST( Class )
  \ingroup unittest
  \hideinitializer

  Use this macro in the test \em runner to make the test exported with
  KDAB_EXPORT_UNITTEST() with the same \a Class argument available to
  be run. This is a work-around for problems with static linking.

  \sa KDAB_IMPORT_UNITTEST_SIMPLE()
*/

/*!
  \def KDAB_IMPORT_UNITTEST_SIMPLE( Class )
  \ingroup unittest
  \hideinitializer

  This is the same as KDAB_IMPORT_UNITTEST(), except that it mangles
  the \a Class argument the same way KDAB_UNITTEST_SIMPLE() does. If
  your test does not use KDAB_UNITTEST_SIMPLE(), use
  KDAB_IMPORT_UNITTEST() instead.
*/

/*!
  \def KDAB_UNITTEST_SIMPLE( Class, Group )
  \ingroup unittest
  \hideinitializer

  This is a convenience macro that you may use when your class under
  test is so small that you can put all your checks into the \link
  KDUnitTest::Test::run() run()\endlink method.

  It defines and exports class \a Class into group \a Group. All you
  need to do is add the checks:
  \code
  KDAB_UNITTEST_SIMPLE( QString, "QtCore" ) {
      QString str;
      assertTrue( str.isNull() );
      assertTrue( str.isEmpty() );
      assertEqual( str.length(), 0 );
      // ...
  }
  \endcode
*/

/*!
  \def KDAB_SCOPED_UNITTEST_SIMPLE( Namespace, Class, Group )
  \ingroup unittest
  \hideinitializer

  Same as KDAB_UNITTEST_SIMPLE(), but for namespaced classes.
*/

#endif // KDAB_NO_UNIT_TESTS
