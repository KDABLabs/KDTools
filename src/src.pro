TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS += KDUnitTest
SUBDIRS += KDToolsCore
SUBDIRS += KDToolsGui
# KDUpdater needs Qt >= 4.4
contains($$list($$[QT_VERSION]), 4.[4-9].*):SUBDIRS += KDUpdater
