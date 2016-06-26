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

#include "kdupdaterufcompressor.h"

#include <QFile>
#include <QDir>
#include <iostream>
#include <cstdlib>

int main(int argc, char** argv)
{
    if( argc != 2 )
    {
        std::cerr << "Usage: " << argv[0] <<
                     " <SourceDir>\n Generates "
                     "SourceDir.kvz file\n";
        return EXIT_FAILURE;
    }

    const QString srcDir = QFile::decodeName( argv[1] );

    if( !QDir(srcDir).exists() )
    {
        std::cerr << argv[1] << " - Directory does not exist\n";
        return EXIT_FAILURE;
    }

    QString fileName = QDir(srcDir).dirName();
    if(fileName.isEmpty())
        fileName = QLatin1String( "CompressedUpdateFile" );

    const QString zipFile = QString::fromLatin1("%1.kvz").arg(fileName);

    qDebug("Source Directory = %s", qPrintable(srcDir));
    qDebug("Compressed file  = %s", qPrintable(zipFile));

    KDUpdater::UFCompressor compressor;
    compressor.setFileName( zipFile );
    compressor.setSource( srcDir );
    if ( !compressor.compress() ) {
        std::cerr << "Creating " << argv[1] << " failed: "
                  << qPrintable( compressor.errorString() ) << std::endl;
        return EXIT_FAILURE;
    }

    std::cerr << "Created " << argv[1] << std::endl;
    return EXIT_SUCCESS;
}
