TEMPLATE = subdirs
CONFIG += ordered
# KDUpdater needs Qt >= 4.4
contains($$list($$[QT_VERSION]), 4.[4-9].*):SUBDIRS += ufcreator ufextractor

