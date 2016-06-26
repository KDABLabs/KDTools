TEMPLATE = lib

CONFIG += plugin kdtools
KDTOOLS = gui core

# Qt5 support
contains($$list($$[QT_VERSION]), 5.[0-9].*) {
#QT += designer
}else:{
CONFIG += designer
}

TARGET = kdtools_designer_plugin

include( ../stage.pri )
include(../features/kdtools.prf)

RESOURCES = designer.qrc

SOURCES = plugin.cpp

target.path = "$$KDTOOLS_PLUGINS_PREFIX/designer"

INSTALLS += target

exists( templates ) {

	templates.path =$$[QT_INSTALL_PREFIX]/bin/templates # sic!
	templates.files = templates/*.ui

	INSTALLS += templates

}
