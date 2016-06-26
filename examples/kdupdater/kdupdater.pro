TEMPLATE    = subdirs
CONFIG      += ordered
SUBDIRS     += simpleexample
contains(QT_CONFIG,script) {
        SUBDIRS += compatexample
        contains(QT_CONFIG,webkit):SUBDIRS += firmwaredemo
}
