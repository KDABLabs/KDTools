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

#ifndef __KDTOOLS_KDUPDATERUPDATESINFO_P_H__
#define __KDTOOLS_KDUPDATERUPDATESINFO_P_H__

#include "kdupdater.h"

#include <QtCore/QSharedDataPointer>
#include <QtCore/QString>
#include <QtCore/QMap>
#include <QtCore/QVariant>
#include <QtCore/QVector>

// Classes and structures in this header file are for internal use only.
// They are not a part of the public API

namespace KDUpdater
{
    struct UpdateFileInfo
    {
        UpdateFileInfo()
            : compressedSize( 0 ),
              uncompressedSize( 0 )
        {
        }
        QString arch;
        QString platformRegEx;
        QString fileName;
        QByteArray sha1sum;
        quint64 compressedSize;
        quint64 uncompressedSize;
    };

    struct UpdateInfo
    {
        UpdateInfo()
            : type( 0 )
        {
        }
        int type;
        QMap<QString, QVariant> data;
        QVector<UpdateFileInfo> updateFiles;
    };

    class UpdatesInfo
    {
    public:
        enum Error
        {
            NoError=0,
            NotYetReadError,
            CouldNotReadUpdateInfoFileError,
            InvalidXmlError,
            InvalidContentError
        };

        UpdatesInfo();
        ~UpdatesInfo();

        bool isValid() const;
        QString errorString() const;
        Error error() const;

        void setFileName(const QString& updateXmlFile);
        QString fileName() const;

        QString targetName() const;
        QString targetVersion() const;
        int compatLevel() const;

        int updateInfoCount( int type=AllUpdate ) const;
        UpdateInfo updateInfo(int index) const;
        QVector<UpdateInfo> updatesInfo( int type=AllUpdate, int compatLevel=-1 ) const;

    private:
        class Private;
        QSharedDataPointer< Private > d;
};
}

#endif
