include( ../stage.pri )

TEMPLATE = app
TARGET = ufcreator
DEPENDPATH += . ../../src/KDUpdater  ../../src
INCLUDEPATH += . ../../include ../../src/KDUpdater ../../src
QT -= gui webkit xml network
CONFIG += console
macx: CONFIG -= app_bundle

DESTDIR = $$KDTOOLS_BASE/bin

SOURCES     += main.cpp \
           ../../src/KDUpdater/kdupdaterufcompresscommon.cpp

HEADERS += kdupdaterufcompressor.h \
           ../../src/KDUpdater/kdupdaterufcompresscommon_p.h

SOURCES += kdupdaterufcompressor.cpp


