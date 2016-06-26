TEMPLATE = lib

CONFIG -= unittests qt moc

INCLUDEPATH += $$[QT_INSTALL_HEADERS] # needed for qglobal.h
macx:QMAKE_CXXFLAGS += -F$$[QT_INSTALL_LIBS]

CONFIG -= hide_symbols

HEADERS += \
        kdunittestglobal.h \
        kdunittest_static_export.h \
	test.h \
	testregistry.h \

#
SOURCES += \
	test.cpp \
	testregistry.cpp \

#
include( ../stage.pri )


