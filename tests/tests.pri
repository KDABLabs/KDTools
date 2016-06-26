macx:CONFIG -= app_bundle

DESTDIR = $$KDTOOLS_BASE/tests/bin
INCLUDEPATH *= $$KDTOOLS_BASE/include
QT += xml network
CONFIG += kdtools qtestlib console
KDTOOLS += updater

KDTOOLS_BASE_ESCAPED = $$replace(KDTOOLS_BASE, \\\, /)
DEFINES += KDTOOLS_BASE="\"\\\"$$KDTOOLS_BASE_ESCAPED\\\"\""

# qtest.h in 4.5 is not QT_NO_CAST_FROM_BYTEARRAY-clean
contains( $$list($$[QT_VERSION]), 4.5.* ):DEFINES -= QT_NO_CAST_FROM_BYTEARRAY

kdupdaterguiwebview:QT += webkit
kdupdaterguitextbrowser:QT += gui

test.target = test
test.commands = ./$(TARGET)
test.depends = $(TARGET)
QMAKE_EXTRA_TARGETS += test

unix {
    test_with_valgrind.target = "test-with-valgrind"
    test_with_valgrind.commands = valgrind ./$(TARGET)
    test_with_valgrind.depends = $(TARGET)
    QMAKE_EXTRA_TARGETS += test_with_valgrind
}

exists( g++.pri ):include( g++.pri )

