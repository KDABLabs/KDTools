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

#include "kdupdaterpackagesview.h"
#include "kdupdaterpackagesmodel.h"

/*!
   \ingroup kdupdater
   \class KDUpdater::PackagesView kdupdaterpackagesview.h KDUpdaterPackagesView
   \brief A widget that can show packages contained in \ref KDUpdater::PackagesInfo

   \ref KDUpdater::PackagesInfo, associated with \ref KDUpdater::Target, contains
   information about all the packages installed in the target. This widget helps view the packages
   in a list.

   \image html packagesview.jpg

   To use this widget, just create an instance and pass to \ref setPackageInfo() a pointer to
   \ref KDUpdater::PackagesInfo whose information you want this widget to show.
*/

using namespace KDUpdater;

class PackagesView::Private
{
public:
    Private( PackagesView* qq )
        : q( qq )
    {
    }

private:
    PackagesView* const q;

public:
    PackagesModel model;
};

/*!
   Constructor.
*/
PackagesView::PackagesView(QWidget* parent)
    : QTreeView( parent ),
      d ( new Private( this ) )
{
    setModel( &d->model );
    setRootIsDecorated( false );
}

/*!
   Destructor
*/
PackagesView::~PackagesView()
{
}

/*!
   Sets the package info whose information this widget should show.

   \code
   KDUpdater::Target target;

   KDUpdater::PackagesView packageView;
   packageView.setPackageInfo( target.packagesInfo() );
   packageView.show();
   \endcode

*/
void PackagesView::setPackageInfo( const PackagesInfo* packagesInfo )
{
    d->model.setPackageInfo( packagesInfo );
}

/*!
   Returns a pointer to the package info whose information this widget is showing.
*/
const PackagesInfo* PackagesView::packagesInfo() const
{
    return d->model.packagesInfo();
}
