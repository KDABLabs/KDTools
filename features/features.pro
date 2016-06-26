# This file was generated automatically.
# Please edit generate-configure.sh/autogen.py rather than this file.

TEMPLATE = subdirs

features.path = $$KDTOOLS_INSTALL_PREFIX/mkspecs/features
features.files = *.prf

INSTALLS += features

OTHER_FILES = *.prf

include( ../stage.pri )
