TEMPLATE    = app

TARGET      = FileDownloaderTest

include( ../stage.pri )
include(../../features/kdtools.prf)

QT	    += network

SOURCES     += main.cpp
FORMS       += filedownloadmonitor.ui
