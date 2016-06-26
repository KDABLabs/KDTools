TEMPLATE    = lib

QT          += xml network
QT -= gui
CONFIG += kdtools per_platform_sources
KDTOOLS += core

# Input
INSTALLHEADERS = \
           kdupdater.h \
           kdupdaterpackagesinfo.h \
           kdupdaterapplication.h \
           kdupdatertarget.h \
           kdupdaterupdatesourcesinfo.h \
           kdupdaterupdate.h \
           kdupdaterupdateoperation.h \
           kdupdaterupdateoperationfactory.h \
           kdupdaterupdatefinder.h \
           kdupdaterupdateinstaller.h \
           kdupdatertask.h \
           kdupdaterfiledownloader.h \
           kdupdaterfiledownloaderfactory.h \
           kdupdaterpackagesmodel.h \
           kdupdaterupdatesourcesmodel.h \
           kdupdaterupdatesmodel.h \

PRIVATEHEADERS = kdupdaterfiledownloader_p.h \
                 kdupdaterupdateoperations_p.h \
                 kdupdaterupdatesinfo_p.h \

SOURCES += kdupdaterpackagesinfo.cpp \
           kdupdaterapplication.cpp \
           kdupdatertarget.cpp \
           kdupdaterupdatesourcesinfo.cpp \
           kdupdaterupdate.cpp \
           kdupdaterfiledownloader.cpp \
           kdupdaterfiledownloaderfactory.cpp \
           kdupdaterupdateoperation.cpp \
           kdupdaterupdateoperations.cpp \
           kdupdaterupdateoperationfactory.cpp \
           kdupdaterupdatesinfo.cpp \
           kdupdaterupdatefinder.cpp \
           kdupdaterupdateinstaller.cpp \
           kdupdatertask.cpp \
           kdupdaterpackagesmodel.cpp \
           kdupdaterupdatesourcesmodel.cpp \
           kdupdaterufcompresscommon.cpp \
           kdupdaterupdatesmodel.cpp \

PRIVATEHEADERS += kdupdaterufcompresscommon_p.h \
           kdupdaterufuncompressor_p.h

SOURCES += kdupdaterufuncompressor.cpp

kdupdatergui {
    SOURCES +=        kdupdaterupdatesdialog.cpp \
                      kdupdaterpackagesview.cpp \
                      kdupdaterupdatesourcesview.cpp

    INSTALLHEADERS += kdupdaterupdatesdialog.h \
                      kdupdaterpackagesview.h \
                      kdupdaterupdatesourcesview.h

    kdupdaterguitextbrowser {
        DEFINES += KDUPDATERGUITEXTBROWSER KDUPDATERVIEW=QTextBrowser
        QT += gui
    }
    kdupdaterguiwebview {
        DEFINES += KDUPDATERGUIWEBVIEW KDUPDATERVIEW=QWebView
        QT += gui webkit
    }
    FORMS   +=        addupdatesourcedialog.ui \
                      updatesdialog.ui
}

HEADERS = $$INSTALLHEADERS $$PRIVATEHEADERS

include(../stage.pri)
include(../../features/kdtools.prf)
