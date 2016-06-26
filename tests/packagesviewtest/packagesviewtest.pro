CONFIG      += qtestlib console
TARGET      = PackagesViewTest

include( ../stage.pri )

LIBS        += $$KD_UPDATER_LIBS

TEMPLATE    = app
SOURCES     += main.cpp
