TEMPLATE    = app

TARGET      = CompressorTest

include(../stage.pri)
include(../../features/kdtools.prf)

INCLUDEPATH += ../../tools/ufcreator ../../src/KDUpdater

SOURCES     += main.cpp

HEADERS += ../../tools/ufcreator/kdupdaterufcompressor.h \

SOURCES += ../../tools/ufcreator/kdupdaterufcompressor.cpp \
