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

#ifndef __KDTOOLS__GUI__KDSHORTCUTBAR_H__
#define __KDTOOLS__GUI__KDSHORTCUTBAR_H__

#include <KDToolsCore/kdtoolsglobal.h>
#include <KDToolsCore/pimpl_ptr.h>

#if QT_VERSION < 0x050000
#include <QtGui/QAbstractButton>
#else
#include <QtWidgets/QAbstractButton>
#endif

QT_BEGIN_NAMESPACE
class QIcon;
class QMenu;
QT_END_NAMESPACE

class KDShortcut;
class KDShortcutSplitterHandle;

class KDTOOLSGUI_EXPORT KDShortcutBar : public QWidget {
    Q_OBJECT
    friend class KDShortcut;
public:
    enum WidgetPlacement { WidgetAbove, WidgetBelow };

    explicit KDShortcutBar( QWidget* parent = 0 );
    virtual ~KDShortcutBar();

    void addShortcut( KDShortcut* );
    void insertShortcut( KDShortcut* cat, KDShortcut* before );
    void removeShortcut( KDShortcut* );

    int numShortcuts() const;
    KDShortcut* shortcutAt(int) const;

    KDShortcut* currentShortcut() const;

    void setWidgetPlacement( WidgetPlacement );
    WidgetPlacement widgetPlacement() const;

    int numShortcutsVisible() const;

    QByteArray saveState() const;
    bool restoreState( const QByteArray& );

    QSize iconSize() const;
    void setIconSize( const QSize& );

Q_SIGNALS:
    void shortcutSelected( KDShortcut* );
public Q_SLOTS:
    void setCurrentShortcut( KDShortcut* );
    void showMoreShortcuts();
    void showFewerShortcuts();

protected:
    int buttonsHeightIncrement() const;

private:
    class Private;
    friend class Private;
    kdtools::pimpl_ptr<Private> d;
};

class KDTOOLSGUI_EXPORT KDShortcut : public QAbstractButton {
    Q_OBJECT
    Q_PRIVATE_SLOT( d, void slotPressed() )
    Q_PRIVATE_SLOT( d, void slotReleased() )
    friend class ::KDShortcutBar;
    friend class ::KDShortcutBar::Private;
public:
    explicit KDShortcut( QWidget* widget, QWidget* parent = 0 );
    virtual ~KDShortcut();

    KDShortcutBar* shortcutBar() const;
    QWidget* widget() const;

    void setStatusIcon( const QIcon& icon );
    QIcon statusIcon() const;

    bool isCurrent() const;


    void setShortcutVisible( bool );
    bool isShortcutVisible() const;

    void setMenu( QMenu* );
    virtual QMenu* menu() const;

    void showMenu();

    QSize sizeHint() const KDAB_OVERRIDE;

Q_SIGNALS:
    void pressed( KDShortcut* );

protected:
    void paintEvent(QPaintEvent*) KDAB_OVERRIDE;
    void timerEvent(QTimerEvent*) KDAB_OVERRIDE;

private:
    class Private;
    friend class Private;
    kdtools::pimpl_ptr<Private> d;
};

#endif /* __KDTOOLS__GUI__KDSHORTCUTBAR_H__ */

