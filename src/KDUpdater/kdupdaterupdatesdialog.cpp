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

#include "kdupdaterupdatesdialog.h"
#include "kdupdaterpackagesinfo.h"
#include "kdupdaterupdate.h"
#include "kdupdaterupdatesmodel.h"
#include "kdupdatertarget.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QHash>
#include <QtCore/QSet>

#if defined( KDUPDATERGUIWEBVIEW )
#include <QtWebKit/QWebView>
#elif defined( KDUPDATERGUITEXTBROWSER )
#include <QtGui/QTextBrowser>
#endif

#include "ui_updatesdialog.h"

/*!
   \ingroup kdupdater
   \class KDUpdater::UpdatesDialog kdupdaterupdatesdialog.h KDUpdaterUpdatesDialog
   \brief A dialog that let the user chooses which updates he wants to install

   After \ref KDUpdater::UpdateFinder class finds all updates available for an target,
   this dialog can be used to help the user select which update he wants to install.

   Alternatively, you can use an own dialog containing an item view using \ref KDUpdater::UpdatesModel.

   Usage:
   \code
    QList<KDUpdater::Update*> updates = updateFinder.updates();

    KDUpdater::UpdatesDialog updatesDialog(this);
    updatesDialog.setUpdates(updates);

    if( updatesDialog.exec() != QDialog::Accepted )
    {
        qDeleteAll(updates);
        updates.clear();
        return;
    }

    QList<KDUpdater::Update*> reqUpdates;
    for(int i=0; i<updates.count(); i++)
    {
        if( !updatesDialog.isUpdateAllowed(updates[i]) )
            continue;
        reqUpdates.append(updates[i]);
    }
    \endcode
*/

using namespace KDUpdater;

/*!
 \internal
 */
class UpdatesDialog::Private
{
    Q_DECLARE_TR_FUNCTIONS(KDUpdater::Private)

public:
    explicit Private( UpdatesDialog* qq )
        : q( qq ),
        currentUpdate( -1 )
    {
    }

private:
    UpdatesDialog* const q;

public:
    UpdatesModel model;

    Ui::UpdatesDialog ui;

    int currentUpdate;

    void setCurrentUpdate(int index);

    QString packageDescription( const Update* update ) const;
    QString compatDescription( const Update* update ) const;
    void slotStateChanged();
    void slotPreviousClicked();
    void slotNextClicked();
};

/*!
   Constructor.
*/
UpdatesDialog::UpdatesDialog(QWidget *parent)
    : QDialog(parent),
      d( new Private( this ) )
{
    d->ui.setupUi(this);

    connect(d->ui.packageUpdateCheckBox, SIGNAL(stateChanged(int)),
            this, SLOT(slotStateChanged()));
    connect(d->ui.nextPackageButton, SIGNAL(clicked()),
            this, SLOT(slotNextClicked()));
    connect(d->ui.previousPackageButton, SIGNAL(clicked()),
            this, SLOT(slotPreviousClicked()));
}


/*!
   Destructor.
*/
UpdatesDialog::~UpdatesDialog()
{
}


/*!
   Sets the list of updates available to the user.
*/
void UpdatesDialog::setUpdates(const QList<Update*> &updates)
{
    d->model.setUpdates( updates );

    d->ui.packageSwitchBar->setVisible( d->model.rowCount() > 1 );

    if( d->model.rowCount() == 0 )
    {
        d->ui.descriptionLabel->setText( tr( "<b>No update available...</b>" ) );
        d->ui.descriptionLabel->setFixedSize( d->ui.descriptionLabel->sizeHint() );
        d->ui.releaseNotesGroup->hide();
        d->ui.pixmapLabel->hide();
    } 

    d->ui.totalPackageLabel->setText( QString::number( d->model.rowCount() ) );
    d->setCurrentUpdate( 0 );
}


/*!
   returns the list of updates available to the user.
*/
QList<Update*> UpdatesDialog::updates() const
{
    return d->model.updates();
}


/*!
   Returns true if the update needs to be installed.
*/
bool UpdatesDialog::isUpdateAllowed( const Update* update ) const
{
    return d->model.selectedUpdates().contains( const_cast< Update* >( update ) );
}

void UpdatesDialog::Private::slotStateChanged()
{
    model.setData( model.index( currentUpdate, 0 ), ui.packageUpdateCheckBox->checkState(), Qt::CheckStateRole );
}

void UpdatesDialog::Private::slotPreviousClicked()
{
    setCurrentUpdate( currentUpdate - 1 );
}

void UpdatesDialog::Private::slotNextClicked()
{
    setCurrentUpdate( currentUpdate + 1 );
}

void UpdatesDialog::Private::setCurrentUpdate( int index )
{
    if( model.rowCount() == 0 )
    {
        currentUpdate = -1;
        return;
    }

    if( index < 0 || index >= model.rowCount() ) 
        return;

    currentUpdate = index;

    const Update* const update = qVariantValue< Update* >( model.index( currentUpdate, 0 ).data( Qt::UserRole ) );

    QString description;

    switch ( update->type() ) {
    case PackageUpdate:
    case NewPackage:
        description = packageDescription( update );
        break;
    case CompatUpdate:
        description = compatDescription( update );
        break;
    default:
        description = tr( "<unknown>" );
    }

    ui.descriptionLabel->setText(description);
    ui.descriptionLabel->setMinimumHeight(ui.descriptionLabel->heightForWidth(400));

    ui.packageUpdateCheckBox->setCheckState( static_cast< Qt::CheckState >( model.index( currentUpdate, 0 ).data( Qt::CheckStateRole ).toInt() ) );

    ui.currentPackageLabel->setText( QString::number( index + 1 ) );
    ui.nextPackageButton->setEnabled( index != model.rowCount() - 1 );
    ui.previousPackageButton->setEnabled( index != 0 );

    const QDir appdir( update->target()->directory() );
    if (update->data( QLatin1String( "ReleaseNotes" ) ).isValid()) {
        ui.releaseNotesGroup->show();
#if defined( KDUPDATERGUIWEBVIEW )
        ui.releaseNotesView->setUrl( update->data( QLatin1String( "ReleaseNotes" ) ).toUrl() );
#elif defined( KDUPDATERGUITEXTBROWSER )
        ui.releaseNotesView->setSource( update->data( QLatin1String( "ReleaseNotes" ) ).toUrl());
#endif
    }
    else {
        ui.releaseNotesGroup->hide();
    }
}


QString UpdatesDialog::Private::packageDescription( const Update* update ) const
{
    const PackagesInfo * const packages = update->target()->packagesInfo();
    const PackageInfo info = packages->packageInfo(
        packages->findPackageInfo(update->data( QLatin1String( "Name" ) ).toString()));

    const QDir appdir( update->target()->directory() );
    const QPixmap pixmap(appdir.filePath(info.pixmap));
    if (!pixmap.isNull()) {
        ui.pixmapLabel->setPixmap(pixmap.scaled(96, 96));
    }


    QString description = tr("<b>A new package update is available for %1!</b><br/><br/>"
                             "The package %2 %3 is now available -- you have version %4")
                          .arg(packages->targetName(),
                               update->data( QLatin1String( "Name" ) ).toString(),
                               update->data( QLatin1String( "Version" ) ).toString(),
                               info.version);

    if (!info.title.isEmpty() || !info.description.isEmpty() ) {
        description += QLatin1String( "<br/><br/>" );
        description += tr("<b>Package Details:</b>" );
        if ( !info.title.isEmpty() ) {
            description += tr( "<br/><i>Title:</i> %1" ).arg( info.title );
        }
        if ( !info.description.isEmpty() ) {
            description += tr( "<br/><i>Description:</i> %1" ).arg( info.description );
        }
    }

    if ( update->data( QLatin1String( "Description" ) ).isValid() ) {
        description += QLatin1String( "<br/><br/>" );
        description += tr( "<b>Update description:</b><br/>%1" )
                       .arg( update->data( QLatin1String( "Description" ) ).toString() );
    }
    return description;
}

QString UpdatesDialog::Private::compatDescription( const Update* update ) const
{
    const PackagesInfo* const packages = update->target()->packagesInfo();

    QString description = tr("<b>A new compatibility update is available for %1!</b><br/><br/>"
                             "The compatibility level %2 is now available -- you have level %3")
                          .arg(packages->targetName(),
                               QString::number(update->data( QLatin1String( "CompatLevel" ) ).toInt()),
                               QString::number(packages->compatLevel()));

    if ( update->data( QLatin1String( "Description" ) ).isValid() ) {
        description += QLatin1String( "<br/><br/>" );
        description += tr( "<b>Update description:</b> %1" )
                       .arg( update->data( QLatin1String( "Description" ) ).toString() );
    }
    return description;
}

#include "moc_kdupdaterupdatesdialog.cpp"
