/****************************************************************************
** Copyright (C) 2001-2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com.
** All rights reserved.
**
** This file is part of the KD Tools library.
**
** Licensees holding valid commercial KD Tools licenses may use this file in
** accordance with the KD Tools Commercial License Agreement provided with
** the Software.
**
** This file may be distributed and/or modified under the terms of the
** GNU Lesser General Public License version 2.1 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.LGPL.txt included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/

#include "helpdialog.h"

#include <QVBoxLayout>
#if defined( KDUPDATERGUIWEBVIEW )
#include <QtWebKit/QWebView>
#elif defined( KDUPDATERGUITEXTBROWSER )
#include <QtGui/QTextBrowser>
#endif
#include <QDebug>

HelpDialog::HelpDialog( QWidget* parent)
    : QDialog( parent )
{
    QVBoxLayout* layout = new QVBoxLayout;
    setLayout( layout );

    KDUPDATERVIEW* webView = new KDUPDATERVIEW( this );
    layout->addWidget( webView );

    const QString text = tr(
        "<h3>Presentation</h3> \
        This application demonstrates some aspects of the KDAB updater component.<br/> \
        Initially you can add target items (<a href=#L4RAction>Line 4 Receiver</a> or <a href=#PSMAction>PSM</a> devices) that use some current firmware version, to the table. <br/> \
        You can see the current compatibility level and firmware version known to the application in the status bar. The source used for finding updates is available when clicking on the button <a href=#SourcesAction>Update Sources</a>.\
        <h3>Step 1: add some items</h3> \
        Add some items to the table, using the <a href=#L4RAction>Add Line 4 Receiver</a> or <a href=#PSMAction>Add PSM</a> button. \
        <h3>Step 2: update the firmware</h3> \
        Update the firmware information by clicking on the <a href=#UpdateAction>Update</a> button : a dialog appears that lets you download and install the version <b>1.1</b> for the package <b>Firmware</b>.<br/> \
        Once that is done, the status bar is updated with the new firmware version.<br/> \
        To update the devices in the table: <a href=#SelectAllAction>select them all</a> and click on the <a href=#UploadAction>Upload Firmwares</a> button. \
        <h3>Step 3: change the compatibility level</h3> \
        The current compatibility level is <b>1</b>. To change that, click on the <a href=#CompatUpdateAction>Update Compat</a> button : a dialog appears that lets you change to a new compat level <b>2</b>, if updates at that level are available.<br/>\
        Once it's done, the status bar is updated with the new compat level and you should see a new column in the table and the application's style changed.\
        <h3>Step 4: update the firmware (one more time)</h3> \
        Now that you changed the compatibility level, you can update the firmware one more time by clicking on the <a href=#UpdateAction>Update</a> button: a dialog appears that lets you install the version <b>2.0</b> for the package <b>Firmware</b><br/> \
        This update wasn't available the first time because it needs the compatibility level <b>2</b>." );

    webView->setHtml( text );
#if defined( KDUPDATERGUIWEBVIEW )
    webView->page()->setLinkDelegationPolicy( QWebPage::DelegateAllLinks );
#endif
    connect( webView, SIGNAL(linkClicked(QUrl)),
             this, SLOT(clickLink(QUrl)) );

    setWindowTitle( tr("Help FirmwareDemoHttp") );
    resize( 480, 480 );
}

HelpDialog::~HelpDialog()
{

}

void HelpDialog::clickLink( const QUrl &url )
{
    emit linkClicked( url.fragment() );
}


