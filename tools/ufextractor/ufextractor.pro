include( ../stage.pri )

TEMPLATE = app
TARGET = ufextractor
DEPENDPATH += . ../../src/KDUpdater ../../src
INCLUDEPATH += . ../../include ../../src/KDUpdater ../../src
QT -= gui webkit xml network
CONFIG += console
macx:CONFIG -= app_bundle

DESTDIR = $$KDTOOLS_BASE/bin

SOURCES     += main.cpp \
           ../../src/KDUpdater/kdupdaterufcompresscommon.cpp

HEADERS += ../../src/KDUpdater/kdupdaterufcompresscommon_p.h \
           ../../src/KDUpdater/kdupdaterufuncompressor_p.h \
           ../../src/KDToolsCore/kdsavefile.h \
           ../../src/KDToolsCore/kdmetamethoditerator.h

SOURCES += ../../src/KDUpdater/kdupdaterufuncompressor.cpp \
           ../../src/KDToolsCore/kdsavefile.cpp \
           ../../src/KDToolsCore/kdmetamethoditerator.cpp
