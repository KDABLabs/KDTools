TEMPLATE    = subdirs
CONFIG      += ordered
TARGET      = tests

# KDUpdater needs Qt >= 4.4
contains($$list($$[QT_VERSION]), 4.[4-9].*) {

TESTDIRS    = updateclienttest \
              filedownloadertest \
              compressortest \
              versioncomparefntest \
              updatefindertest \
              updateinstallertest \
              updateoperationstest \
              propertychangetest

kdupdatergui: TESTDIRS += packagesviewtest \
                          updatesourcesviewtest

SUBDIRS     += $${TESTDIRS}

} # contains($$list($$[QT_VERSION]), 4.[4-9].*)

test.target=test
unix:!macx {
    LIB_PATH=../../lib:\$\$LD_LIBRARY_PATH
    test.commands=success=true; for d in $${TESTDIRS}; do ( cd "\$$d" && LD_LIBRARY_PATH=$$LIB_PATH && $(MAKE) test ) || success=false; done; \$$success
}
unix:macx {
    LIB_PATH=../../lib:\$\$DYLD_LIBRARY_PATH
    test.commands=success=true; for d in $${TESTDIRS}; do ( cd "\$$d" && export DYLD_LIBRARY_PATH=$$LIB_PATH && $(MAKE) test ) || success=false; done; \$$success
}
win32:test.commands=\"$${KDTOOLS_BASE}/runtests.bat\" $${TESTDIRS}
test.depends = first

QMAKE_EXTRA_TARGETS += test

unix {
    test_with_valgrind.target="test-with-valgrind"
    !macx {
        LIB_PATH=../../lib:\$\$LD_LIBRARY_PATH
        test_with_valgrind.commands=success=true; for d in $${TESTDIRS}; do ( cd "\$$d" && LD_LIBRARY_PATH=$$LIB_PATH && $(MAKE) test-with-valgrind ) || success=false; done; \$$success
    }
    macx {
        LIB_PATH=../../lib:\$\$DYLD_LIBRARY_PATH
        test_with_valgrind.commands=success=true; for d in $${TESTDIRS}; do ( cd "\$$d" && export DYLD_LIBRARY_PATH=$$LIB_PATH && $(MAKE) test-with-valgrind ) || success=false; done; \$$success
    }
    test_with_valgrind.depends=first
    QMAKE_EXTRA_TARGETS += test_with_valgrind
}
