TEMPLATE = app
LIBS += $$KD_UPDATER_LIBS
KDTOOLS += core \
    updater
include( ../stage.pri )
SOURCES += ../main.cpp \
    ../mainwindow.cpp \
    ../updaterdialog.cpp \
    ../updater.cpp
HEADERS += ../mainwindow.h \
    ../updaterdialog.h \
    ../updater.h
DEFINES += APP_VERSION="\\\"1.0\\\""
    contains( CONFIG, out_of_source_build ){
        DESTREPODIR = $$DESTDIR/data/kdupdaterdemo/repository
    }else{
        DESTREPODIR = $$SRCDIR
    }
DEFINES += REPO_DIR="\\\"$$DESTREPODIR\\\""
out_of_source_build:message("OUT OF SOURCE BUILD")
RESOURCES += ../kdupdaterdemo.qrc
