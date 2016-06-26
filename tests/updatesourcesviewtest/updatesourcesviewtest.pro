TEMPLATE    = app

CONFIG      += qtestlib console
TARGET      = UpdateSourcesViewTest

include( ../stage.pri )

LIBS        += $$KD_UPDATER_LIBS

SOURCES     += main.cpp
