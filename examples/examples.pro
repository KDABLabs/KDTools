TEMPLATE = subdirs

SUBDIRS += colorbutton \
           logger \
           timelinewidget \
           propertyview \
           kdshortcutbar \
           logtextwidget \
           kdthreadrunner \
           macwidgets \

# KDUpdater needs Qt >= 4.4
contains($$list($$[QT_VERSION]), 4.[4-9].*) {
    # examples need kdupdatergui:
    kdupdatergui:SUBDIRS += kdupdaterdemo kdupdater
}
