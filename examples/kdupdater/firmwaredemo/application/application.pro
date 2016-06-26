TEMPLATE    = app

QT          += script webkit
TARGET      = FirmwareDemo
kdupdaterguitextbrowser {
    DEFINES += KDUPDATERGUITEXTBROWSER KDUPDATERVIEW=QTextBrowser
    QT += gui
}
kdupdaterguiwebview {
    DEFINES += KDUPDATERGUIWEBVIEW KDUPDATERVIEW=QWebView
    QT += gui webkit
}

include( ../stage.pri )

EXAMPLE_APP_DIR="$$OUT_PWD/appdir"

DEFINES += EXAMPLE_APP_DIR="\\\"$$EXAMPLE_APP_DIR\\\""

out_of_source_build:qmakeCleanAndCopy("$$PWD/appdir", "$$EXAMPLE_APP_DIR")

qmakeEcho("<UpdateSources>", ">", "$$EXAMPLE_APP_DIR/UpdateSources.xml")
qmakeEcho("  <UpdateSource>", ">>", "$$EXAMPLE_APP_DIR/UpdateSources.xml")
qmakeEcho("    <Name>KDABSource</Name>", ">>", "$$EXAMPLE_APP_DIR/UpdateSources.xml")
qmakeEcho("    <Title>KDAB Update Source</Title>", ">>", "$$EXAMPLE_APP_DIR/UpdateSources.xml")
qmakeEcho("    <Description>Update source from KDAB</Description>", ">>", "$$EXAMPLE_APP_DIR/UpdateSources.xml")
qmakeEcho("    <Url>http://www.kdab.net/~nicolas/updateserver</Url>", ">>", "$$EXAMPLE_APP_DIR/UpdateSources.xml")
qmakeEcho("  </UpdateSource>", ">>", "$$EXAMPLE_APP_DIR/UpdateSources.xml")
qmakeEcho("</UpdateSources>", ">>", "$$EXAMPLE_APP_DIR/UpdateSources.xml")

SOURCES     += main.cpp mainwindow.cpp helpdialog.cpp
HEADERS     += mainwindow.h helpdialog.h
