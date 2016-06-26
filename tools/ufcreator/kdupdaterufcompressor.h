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

#ifndef KD_UPDATER_UF_COMPRESSOR_H
#define KD_UPDATER_UF_COMPRESSOR_H

#include <QCoreApplication>

QT_BEGIN_NAMESPACE
class QString;
QT_END_NAMESPACE

namespace KDUpdater
{
    class UFCompressor
    {
        Q_DECLARE_TR_FUNCTIONS(KDUpdater::UFCompressor)

    public:
        UFCompressor();
        ~UFCompressor();

        QString errorString() const;
        
        void setFileName(const QString& fileName);
        QString fileName() const;

        void setSource(const QString& source);
        QString source() const;

        bool compress();

    private:
        struct UFCompressorData;
        UFCompressorData* d;
    };
}

#endif
