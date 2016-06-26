#
# This file was generated automatically.
# Please edit generate-configure.sh/autogen.py rather than this file.
#
PROJECT = kdtools

DEFINES += emit=""
DEFINES += QT_NO_KEYWORDS QT_NO_CAST_TO_ASCII QT_NO_CAST_FROM_ASCII QT_NO_CAST_FROM_BYTEARRAY

!exists("$$OUT_PWD/../stage.pri"):CONFIG += out_of_source_build

INCLUDEPATH += $$PWD/include

TOPLEVEL=$$KDTOOLS_BASE

QMAKE_LIBDIR += $$TOPLEVEL/lib

DEBUG_SUFFIX=""
CONFIG( debug, debug|release ) {
	win32 {
		CONFIG( shared, static|shared ):DEBUG_SUFFIX = d
	} else {
		# the following is only used in Qt versions < 4.2.x:
		!contains($$list($$[QT_VERSION]), 4.[2-9].*) {
			DEBUG_SUFFIX = _debug
		}
	}
}

unittests {
    KDTOOLS += unittest
    CONFIG( static, static|shared ):DEFINES += UNITTESTS_IN_STATIC_LIB
}

contains( TEMPLATE, lib ) {

	VERSION = "2.3.0"

	unittests:DEFINES += $$upper($$TARGET)_UNITTESTS

	linux-g++*:QMAKE_LFLAGS += -Wl,-no-undefined

	CONFIG( shared, static|shared ) {
		DEFINES += $$upper($$PROJECT)_SHARED
		DEFINES += BUILD_SHARED_$$upper($$TARGET)
		win32:DLLDESTDIR= $$TOPLEVEL/bin
	}

	!plugin:DESTDIR = $$TOPLEVEL/lib

	CONFIG( debug, debug|release ) {
		TARGET = $${TARGET}$${DEBUG_SUFFIX}
	}

	!plugin {
		target.path = $$KDTOOLS_INSTALL_PREFIX/lib
		INSTALLS += target
		win32 {
			dlltarget.path = $$KDTOOLS_INSTALL_PREFIX/bin
			INSTALLS += dlltarget
		}
	}

} else:contains( TEMPLATE, subdirs ) {

	!isEmpty( HEADERS ) {
		headers.path = $$KDTOOLS_INSTALL_PREFIX/include/$$TARGET
		headers.files = $$HEADERS

		INSTALLS += headers
	}

} else {

	DESTDIR = $$TOPLEVEL/bin

}

unix {
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
  UI_DIR = .ui
  RCC_DIR = .qrc
}
!unix {
  debug:MODE=debug
  release:MODE=release
  MOC_DIR = $${MODE}/_moc
  OBJECTS_DIR = $${MODE}/_obj
  UI_DIR = $${MODE}/_ui
  RCC_DIR = _qrc
}

exists( g++.pri ):include( g++.pri )
