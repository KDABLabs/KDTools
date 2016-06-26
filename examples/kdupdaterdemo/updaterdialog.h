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

#ifndef UPDATERDIALOG_H
#define UPDATERDIALOG_H

#include <QDialog>

#include <KDToolsCore/pimpl_ptr>
#include <kdtoolsglobal.h>

class UpdaterDialog : public QDialog
{
    Q_OBJECT
    KDAB_DISABLE_COPY( UpdaterDialog );
public:    
    explicit UpdaterDialog( QWidget *parent = 0 );
    ~UpdaterDialog();

private:
    class Private;
    kdtools::pimpl_ptr< Private > d;

};

#endif // UPDATERDIALOG_H
