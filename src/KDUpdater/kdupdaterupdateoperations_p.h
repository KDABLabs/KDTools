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

#ifndef __KDTOOLS_KDUPDATERUPDATEOPERATIONS_P_H__
#define __KDTOOLS_KDUPDATERUPDATEOPERATIONS_P_H__

#include "kdupdaterupdateoperation.h"

#include <QtCore/QCoreApplication> // for Q_DECLARE_TR_FUNCTIONS
#include <QProcess>

namespace KDUpdater
{

    class CopyOperation : public UpdateOperation
    {
        Q_DECLARE_TR_FUNCTIONS( CopyOperation )
    public:
        CopyOperation();
        ~CopyOperation();

        void backup();
        bool performOperation();
        bool undoOperation();
        bool testOperation();
        CopyOperation* clone() const;
    };

    class MoveOperation : public UpdateOperation
    {
        Q_DECLARE_TR_FUNCTIONS( MoveOperation )
    public:
        MoveOperation();
        ~MoveOperation();

        void backup();
        bool performOperation();
        bool undoOperation();
        bool testOperation();
        MoveOperation* clone() const;
    };

    class DeleteOperation : public UpdateOperation
    {
        Q_DECLARE_TR_FUNCTIONS( DeleteOperation )
    public:
        DeleteOperation();
        ~DeleteOperation();

        void backup();
        bool performOperation();
        bool undoOperation();
        bool testOperation();
        DeleteOperation* clone() const;
    };

    class MkdirOperation : public UpdateOperation
    {
        Q_DECLARE_TR_FUNCTIONS( MkdirOperation )
    public:
        MkdirOperation();
        ~MkdirOperation();

        void backup();
        bool performOperation();
        bool undoOperation();
        bool testOperation();
        MkdirOperation* clone() const;
    };

    class RmdirOperation : public UpdateOperation
    {
        Q_DECLARE_TR_FUNCTIONS( RmdirOperation )
    public:
        RmdirOperation();
        ~RmdirOperation();

        void backup();
        bool performOperation();
        bool undoOperation();
        bool testOperation();
        RmdirOperation* clone() const;
    };

    class AppendFileOperation : public UpdateOperation
    {
        Q_DECLARE_TR_FUNCTIONS( AppendFileOperation )
    public:
        AppendFileOperation();
        ~AppendFileOperation();

        void backup();
        bool performOperation();
        bool undoOperation();
        bool testOperation();
        AppendFileOperation* clone() const;
    };

    class PrependFileOperation : public UpdateOperation
    {
        Q_DECLARE_TR_FUNCTIONS( PrependFileOperation )
    public:
        PrependFileOperation();
        ~PrependFileOperation();

        void backup();
        bool performOperation();
        bool undoOperation();
        bool testOperation();
        PrependFileOperation* clone() const;
    };

    class ExecuteOperation : public QObject, public UpdateOperation
    {
        Q_OBJECT
    public:
        ExecuteOperation();
        ~ExecuteOperation();

        void backup();
        bool performOperation();
        bool undoOperation();
        bool testOperation();
        ExecuteOperation* clone() const;

    public Q_SLOTS:
        void cancelOperation();

    private Q_SLOTS:
        void readProcessOutput();

    Q_SIGNALS:
        void outputChanged(const QString &text);

    private:
        QProcess process;
    };

    class UpdatePackageOperation : public UpdateOperation
    {
        Q_DECLARE_TR_FUNCTIONS( UpdatePackageOperation )
    public:
        UpdatePackageOperation();
        ~UpdatePackageOperation();

        void backup();
        bool performOperation();
        bool undoOperation();
        bool testOperation();
        UpdatePackageOperation* clone() const;
    };

    class UpdateCompatOperation : public UpdateOperation
    {
        Q_DECLARE_TR_FUNCTIONS( UpdateCompatOperation )
    public:
        UpdateCompatOperation();
        ~UpdateCompatOperation();

        void backup();
        bool performOperation();
        bool undoOperation();
        bool testOperation();
        UpdateCompatOperation* clone() const;
    };

}

#endif
