TEMPLATE = lib
TARGET = KDToolsGui

CONFIG += kdtools per_platform_sources
KDTOOLS = core

# Qt5 support
contains($$list($$[QT_VERSION]), 5.[0-9].*) {
QT += widgets
}

HEADERS += \
	kdcolorbutton.h \
	kdlogtextwidget.h \
	kdoverridecursor.h \
	kdpaintersaver.h \
        kdemailvalidator.h \
        kdtransformmapper.h \
	kdtimescrollbar.h \
	kdtimelinewidget.h \
	kdproperty.h \
	kdpropertyeditor.h \
	kdpropertymodel.h \
	kdpropertyview.h \
	kdpropertyviewdelegate_p.h \
	kdshortcutsplitterhandle_p.h \
	kdshortcutbar.h \
    macwidget_p.h \
    kdsearchlineedit.h \
    kdhelpbutton.h \
    kdcircularprogressindicator.h \
    kdlockbutton.h \
    kdlistviewtoolbar.h \
    kddoublespinbox.h \

#
SOURCES += \
	kdcolorbutton.cpp \
	kdlogtextwidget.cpp \
	kdoverridecursor.cpp \
	kdpaintersaver.cpp \
        kdemailvalidator.cpp \
        kdtransformmapper.cpp \
	kdtimescrollbar.cpp \
	kdtimelinewidget.cpp \
	kdproperty.cpp \
	kdpropertyeditor.cpp \
	kdpropertymodel.cpp \
	kdpropertyview.cpp \
	kdpropertyviewdelegate_p.cpp \
	kdshortcutsplitterhandle_p.cpp \
	kdshortcutbar.cpp \
    macwidget_p.cpp \
    kdsearchlineedit.cpp \
    kdhelpbutton.cpp \
    kdcircularprogressindicator.cpp \
    kdlockbutton.cpp \
    kdlistviewtoolbar.cpp \
    kddoublespinbox.cpp \
#

macx {
    HEADERS += kdcocoautils.h
    OBJECTIVE_SOURCES += kdcocoautils_mac.mm
    LIBS += -framework Cocoa
}

RESOURCES += KDAB_kdtools_macwidgets_resources.qrc

include( ../stage.pri )
include(../../features/kdtools.prf)
