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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDir>
#include <QMainWindow>
#include <QMap>

QT_BEGIN_NAMESPACE
class QAction;
class QTableWidget;
QT_END_NAMESPACE

namespace KDUpdater {
    class Target;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow( KDUpdater::Target * target, QWidget * parent=0 );
    ~MainWindow();

public Q_SLOTS:
    void triggerAction( const QString& name );

private Q_SLOTS:
    void onAddL4R();
    void onAddPSM();
    void onUploadSelected();
    void onUpdate();
    void onUpdateCompat();
    void showPackages();
    void showSources();

private:
    void loadApplicationData();
    void uploadFirmware(const QString &firmware, int device);
    void displayDevice(int device, int row);
    void updatePackage( bool findCompat );

    KDUpdater::Target * m_target;
    QTableWidget *m_table;
    int m_lastId;
    QDir m_appDir;
    QDir m_devicesDir;
    QMap<QString, QAction*> m_actionMap;
};

#endif
