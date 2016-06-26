TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS += include

exists( src/src.pro ):SUBDIRS += src
exists( tools/tools.pro ):SUBDIRS += tools

unittests:SUBDIRS += unittestrunner
SUBDIRS += features
CONFIG( shared, static|shared ):SUBDIRS += designer
SUBDIRS += examples
SUBDIRS += tests


# forward make test calls to unittestrunner
test.target=test
!unittests {
    test.commands=(cd tests && $(MAKE) test)
} else{
    unix {
        macx:LD_LIB_PATH=DYLD_LIBRARY_PATH
        else:LD_LIB_PATH=LD_LIBRARY_PATH
        LIB_PATH=./lib:\$\$$$LD_LIB_PATH
        test.commands=$$LD_LIB_PATH=$$LIB_PATH ./bin/unittestrunner && ( cd tests && $$LD_LIB_PATH=$$LIB_PATH $(MAKE) test ) || exit 1
    } else {
        test.commands=@bin\unittestrunner && ( cd tests && $(MAKE) test )
    }
}
test.depends = first
QMAKE_EXTRA_TARGETS += test

# forward make test-with-valgrind calls to unittestrunner
unix {
    test_with_valgrind.target="test-with-valgrind"
    !unittests {
        test_with_valgrind.commands=(cd tests && $(MAKE) test-with-valgrind)
    } else{
        macx:LD_LIB_PATH=DYLD_LIBRARY_PATH
        else:LD_LIB_PATH=LD_LIBRARY_PATH
        LIB_PATH=./lib:\$\$$$LD_LIB_PATH
        test_with_valgrind.commands=$$LD_LIB_PATH=$$LIB_PATH valgrind ./bin/unittestrunner && ( cd tests && $$LD_LIB_PATH=$$LIB_PATH $(MAKE) test-with-valgrind ) || exit 1
    }
    test_with_valgrind.depends = first
    QMAKE_EXTRA_TARGETS += test_with_valgrind
}
