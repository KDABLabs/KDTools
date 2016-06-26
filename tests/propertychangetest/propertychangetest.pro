TEMPLATE    = app
CONFIG += kdtools qtestlib console
KDTOOLS += gui
TARGET      = PropertyChangeTest

include( ../stage.pri )
include(../../features/kdtools.prf)

SOURCES     += main.cpp
