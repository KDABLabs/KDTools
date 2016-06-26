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

#include "kdupdaterupdatesourcesview.h"
#include "kdupdaterupdatesourcesinfo.h"
#include "kdupdaterupdatesourcesmodel.h"
#include "ui_addupdatesourcedialog.h"

#include <KDToolsCore/KDAutoPointer>

#include <QMessageBox>
#include <QContextMenuEvent>
#include <QAction>
#include <QMenu>

/*!
   \ingroup kdupdater
   \class KDUpdater::UpdateSourcesView kdupdaterupdatesourcesview.h KDUpdaterUpdateSourcesView
   \brief A widget that helps view and/or edit \ref KDUpdater::UpdateSourcesInfo

   \ref KDUpdater::UpdateSourcesInfo, associated with \ref KDUpdater::Target, contains information
   about all the update sources from which the application can download and install updates.
   This widget helps view and edit update sources information.

   \image html updatesourcesview.jpg

   The widget provides the following slots for editing update sources information
   \ref addNewSource()
   \ref editCurrentSource()
   \ref removeCurrentSource()

   You can include this widget within another form or dialog and connect to these slots which make
   use of an inbuilt dialog box to add/edit update sources. Shown below is a screenshot of the
   inbuilt dialog box.

   \image html editupdatesource.jpg

   Alternatively you can also use your own dialog box and directly update \ref KDUpdater::UpdateSourcesInfo.
   This widget connects to \ref KDUpdater::UpdateSourcesInfo signals and ensures that the data it displays
   is always kept updated.

   The widget provides a context menu using which you can add/remove/edit update sources. Shown below is a
   screenshot of the context menu.

   \image html updatesourcesview_contextmenu.jpg
*/

using namespace KDUpdater;

class EditUpdateSourceDialog::Private
{
public:
    Private( EditUpdateSourceDialog* qq )
        : q( qq )
    {
    }

private:
    EditUpdateSourceDialog* const q;

public:
    Ui::AddUpdateSourceDialog ui;
};

class UpdateSourcesView::Private
{
public:
    Private( UpdateSourcesView* qq ) 
        : q( qq )
    {
    }

private:
    UpdateSourcesView* const q;

public:
    UpdateSourcesModel model;
};

/*!
 Creates a new EditUpdateSourceDialog with \a parent.
*/
EditUpdateSourceDialog::EditUpdateSourceDialog( QWidget* parent )
    : QDialog( parent ),
      d( new Private( this ) )
{
    d->ui.setupUi( this );
    setInfo( UpdateSourceInfo() );
}

/*!
 Destroys the EditUpdateSourceDialog.
*/
EditUpdateSourceDialog::~EditUpdateSourceDialog()
{
}
 
/*!
 Fill the dialog with the values within \a info.
*/
void EditUpdateSourceDialog::setInfo( const UpdateSourceInfo& info )
{
    d->ui.txtName->setText( info.name );
    d->ui.txtTitle->setText( info.title );
    d->ui.txtDescription->setPlainText( info.description ); // FIXME: This should perhaps be setHtml
    d->ui.txtUrl->setText( info.url.toString() );

    setWindowTitle( info == UpdateSourceInfo() ? tr( "Add Update Source" )
                                               : tr( "Edit Update Source" ) );
}

/*!
 Contructs a UpdateSourceInfo from the data the user entered.
*/
UpdateSourceInfo EditUpdateSourceDialog::info() const
{
    UpdateSourceInfo result;
    result.name = d->ui.txtName->text();
    result.title = d->ui.txtTitle->text();
    result.description = d->ui.txtDescription->toPlainText(); // FIXME: This should perhaps be toHtml
    result.url = QUrl( d->ui.txtUrl->text() );
    return result;
}

/*!
 \reimp
*/
void EditUpdateSourceDialog::accept()
{
    if( d->ui.txtName->text().isEmpty() || d->ui.txtUrl->text().isEmpty() )
    {
        QMessageBox::information( this, tr( "Invalid Update Source Info" ),
                                  tr( "A valid update source name and url has to be provided" ) );
    }
    else
    {
        QDialog::accept();
    }
}

/*!
   Constructor
*/
UpdateSourcesView::UpdateSourcesView( QWidget* parent )
    : QTreeView( parent ),
      d( new Private( this ) )
{
    setRootIsDecorated( false );
    setModel( &d->model );
    connect( this, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(editCurrentSource()) );
}

/*!
   Destructor
*/
UpdateSourcesView::~UpdateSourcesView()
{
}

/*!
   Sets the \ref KDUpdater::UpdateSourcesInfo object whose information this widget should show.

   \code
   KDUpdater::Target target;

   KDUpdater::UpdateSourcesView updatesView;
   updatesView.setUpdateSourcesInfo( target.updateSourcesInfo() );
   updatesView.show();
   \endcode
*/
void UpdateSourcesView::setUpdateSourcesInfo( UpdateSourcesInfo* info )
{
    d->model.setUpdateSourcesInfo( info );
}

/*!
   Returns a pointer to the \ref KDUpdater::UpdateSourcesInfo object whose information this
   widget is showing.
*/
UpdateSourcesInfo* UpdateSourcesView::updateSourcesInfo() const
{
    return d->model.updateSourcesInfo();
}

/*!
   Returns the index of the currently selected update source in the widget. You can use this
   index along with \ref KDUpdater::UpdateSourcesInfo::updateSourceInfo() method to get hold
   of the update source info.
*/
int UpdateSourcesView::currentUpdateSourceInfoIndex() const
{
    return currentIndex().row();
}

/*!
   Call this slot to make use of the in-built dialog box to add a new update source. Shown
   below is a screenshot of the in-built dialog box.

   \image html addupdatesource.jpg
*/
void UpdateSourcesView::addNewSource()
{
    KDAutoPointer< EditUpdateSourceDialog > dialog( new EditUpdateSourceDialog( this ) );
    if( dialog->exec() == QDialog::Rejected )
        return;

    d->model.updateSourcesInfo()->addUpdateSourceInfo( dialog->info() );
}

/*!
   Call this slot to delete the currently selected update source.
*/
void UpdateSourcesView::removeCurrentSource()
{
    d->model.removeRow( currentUpdateSourceInfoIndex() );
}

/*!
   Call this slot to edit the currently selected update source, using the in-built edit
   update source dialog box. Shown below is a screenshot of the edit update source dialog
   box.

   \image html editupdatesource.jpg
*/
void UpdateSourcesView::editCurrentSource()
{
    const QModelIndex index = currentIndex();
    if( !index.isValid() )    
        return;

    const UpdateSourceInfo info = qVariantValue< UpdateSourceInfo >( index.data( Qt::UserRole ) );

    KDAutoPointer< EditUpdateSourceDialog > dialog( new EditUpdateSourceDialog( this ) );
    dialog->setInfo( info );
    if( dialog->exec() == QDialog::Rejected )
        return;

    d->model.setData( index, qVariantFromValue( dialog->info() ), Qt::UserRole );
}

#ifndef QT_NO_CONTEXTMENU
/*!
   \internal
*/
void UpdateSourcesView::contextMenuEvent( QContextMenuEvent* e )
{
    const QModelIndex index = indexAt( e->pos() );

    QMenu menu;
    menu.addAction( tr( "&Add Source" ), this, SLOT(addNewSource()) );
    if( index.isValid() )
    {
        menu.addAction( tr( "&Edit Source" ), this, SLOT(editCurrentSource()) );
        menu.addAction( tr( "&Remove Source" ), this, SLOT(removeCurrentSource()) );
    }

    menu.exec( e->globalPos() );
}
#endif // QT_NO_CONTEXTMENU

#include "moc_kdupdaterupdatesourcesview.cpp"
