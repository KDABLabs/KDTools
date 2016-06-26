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

#include "testregistry.h"

#include "test.h"

#include <memory>
#include <iostream>
#include <iomanip>
#include <cassert>

using namespace KDUnitTest;

static std::string make_group_name( const char * group ) {
    assert(  group );
    assert( *group );
    const std::string s = group;
    if ( *s.rbegin() == '/' )
        return s;
    else
        return s + '/';
}

/*!
  \class KDUnitTest::TestRegistry
  \ingroup unittest
  \short A registry for \link KDUnitTest::Test KDUnitTest Tests\endlink

  TestRegistry is the central repository for tests. By way of
  TestFactory, individual tests are added into a hierarchy of groups
  (represented by strings). Tests can then be executed based on their
  group.

  You usually want to use KDUnitTest::Runner, which acts as a \htmlonly fa&ccedil;ade \endhtmlonly \latexonly fa\c{c}ade \endlatexonly
  for the TestRegistry.

  TestRegistry is not thread-safe.
*/

TestRegistry::TestRegistry()
    : mTests()
{

}

TestRegistry::~TestRegistry() {}

TestRegistry * TestRegistry::mSelf = 0;

/*!
  Returns the unique TestRegistry instance.
*/
// static
TestRegistry * TestRegistry::instance() {
    if ( !mSelf )
        mSelf = new TestRegistry;
    return mSelf;
}

/*!
  Deletes the unique TestRegistry instance.
*/
// static
void TestRegistry::deleteInstance() {
    delete mSelf; mSelf = 0;
}

/*!
  Registers test factory \a tf in group \a group.
  \pre tf != 0
  \pre group && *group
  Do not use this function directly. Use GenericFactory instead.
*/
void TestRegistry::registerTestFactory( const TestFactory * tf, const char * group ) {
    assert( tf );
    mTests[ make_group_name( group ) ].push_back( tf );
}

unsigned int TestRegistry::runTest( const TestFactory * tf ) {
    assert( tf );
    std::auto_ptr<Test> t( tf->create() );
    assert( t.get() );
    std::cerr << "  === \"" << t->name() << "\" ===" << std::endl;
    try {
        t->run();
    } catch ( const std::exception & e ) {
        t->fail( __FILE__, __LINE__-2 ) << "Caught exception escaping run(): " << e.what() << std::endl;
    } catch ( ... ) {
        t->fail( __FILE__, __LINE__-4 ) << "Caught unknown exception escaping run()" << std::endl;
    }
    std::cerr << "    Succeeded: " << t->succeeded() << ";  failed: " << t->failed() << std::endl;
    return t->failed();
}

/*!
  Runs all registered tests in all groups and returns the number of
  failed checks.
*/
unsigned int TestRegistry::run() const {
  unsigned int failed = 0;
  for ( std::map< std::string, std::vector<const TestFactory*> >::const_iterator g = mTests.begin() ; g != mTests.end() ; ++g ) {
    std::cerr << "===== GROUP \"" << g->first.substr( 0, g->first.size() - 1 ) << "\" =========" << std::endl;
    for ( std::vector<const TestFactory*>::const_iterator it = g->second.begin() ; it != g->second.end() ; ++it )
      failed += runTest( *it );
  }
  return failed;
}

static std::pair< std::map< std::string, std::vector<const TestFactory*> >::const_iterator,
                  std::map< std::string, std::vector<const TestFactory*> >::const_iterator >
matching_range( const std::map< std::string, std::vector<const TestFactory*> > & tests, const std::string & group )
{
    assert( !group.empty() );
    assert( *group.rbegin() == '/' );
    std::map< std::string, std::vector<const TestFactory*> >::const_iterator b, e;
    e = b = tests.lower_bound( group );
    while ( e != tests.end() && e->first.find( group ) == 0 )
        ++e;
    return std::make_pair( b, e );
}

/*!
  Runs all registered tests in group \a group and returns the number
  of failed checks. If no such group exists, returns 1 (one).
  \pre group is not NULL and not empty
*/
unsigned int TestRegistry::run( const char * group ) const {
  assert( group ); assert( *group );
  unsigned int failed = 0;
  const std::pair< std::map< std::string, std::vector<const TestFactory*> >::const_iterator,
                  std::map< std::string, std::vector<const TestFactory*> >::const_iterator >
      p = matching_range( mTests, make_group_name( group ) );
  if ( p.first == p.second ) {
    std::cerr << "ERROR: No such group \"" << group << "\"" << std::endl;
    return 1;
  }
  for ( std::map< std::string, std::vector<const TestFactory*> >::const_iterator g = p.first ; g != p.second ; ++g ) {
      std::cerr << "===== GROUP \"" << g->first.substr( 0, g->first.size() - 1 ) << "\" =========" << std::endl;
      for ( std::vector<const TestFactory*>::const_iterator it = g->second.begin() ; it != g->second.end() ; ++it )
          failed += runTest( *it );
  }
  return failed;
}

/*!
  \class KDUnitTest::Runner
  \ingroup unittest
  \short Test runner

  KDUnitTest::Runner is a convenience \htmlonly fa&ccedil;ade \endhtmlonly \latexonly fa\c{c}ade \endlatexonly
  for TestRegistry. It's
  intended usage is something along the lines of
  \code
  int main( int argc, char * argv[] ) {
      // ...other init...
      KDUnitTest::Runner runner;
      return runner.run( argc > 1 ? argv[1] : 0 ) > 0
          ? EXIT_SUCCESS : EXIT_FAILURE ;
  }
  \endcode
*/

/*!
  \fn Runner::run( const char * group ) const

  If \a group is NULL or empty, runs all tests in all groups.
  Otherwise, runs all tests in group \a group.
*/

#endif // KDAB_NO_UNIT_TESTS
