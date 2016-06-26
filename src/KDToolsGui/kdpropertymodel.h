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

#ifndef __KDTOOLS_GUI_KDPROPERTYMODEL_H__
#define __KDTOOLS_GUI_KDPROPERTYMODEL_H__

#include <KDToolsCore/kdtoolsglobal.h>
#include <KDToolsCore/pimpl_ptr.h>

#include <QtCore/QAbstractItemModel>

class KDPropertyInterface;
class KDPropertyGroupInterface;
class KDPropertyCategory;

class KDTOOLSGUI_EXPORT KDPropertyModel : public QAbstractItemModel {
    Q_OBJECT
public:
    enum Role {
	HelpRole = Qt::UserRole+1,
	ChangedRole,
	SeparatorRole,
	EditorTypeRole,

	MinimumHint,
	MaximumHint,
	InputMaskHint,
	ChoiceOptionsHint,
	EditableHint,
	PasswordModeHint
    };

    explicit KDPropertyModel(QObject* parent = 0);
    ~KDPropertyModel();

    void addProperty( KDPropertyInterface* );
    void removeProperty( KDPropertyInterface* );
    int propertyCount() const;
    KDPropertyInterface* propertyAt(int idx) const;

    bool isFlat() const;

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const KDAB_OVERRIDE;
    QModelIndex parent(const QModelIndex& index) const KDAB_OVERRIDE;

    int rowCount(const QModelIndex& parent) const KDAB_OVERRIDE;
    int columnCount(const QModelIndex& parent) const KDAB_OVERRIDE;
    bool hasChildren(const QModelIndex& parent) const KDAB_OVERRIDE;

    QVariant data(const QModelIndex& index, int role) const KDAB_OVERRIDE;
    bool setData(const QModelIndex& index, const QVariant& value, int role) KDAB_OVERRIDE;

    virtual bool isEditable(const QModelIndex& index) const;
    QModelIndex buddy(const QModelIndex& index) const KDAB_OVERRIDE;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const KDAB_OVERRIDE;

    Qt::ItemFlags flags(const QModelIndex &index) const KDAB_OVERRIDE;

    /* support methods */
    QModelIndex indexOf( KDPropertyInterface* property, int column = 0) const;
    KDPropertyInterface* privateData(const QModelIndex &index) const;
    bool isEnabled(const QModelIndex& index) const;

Q_SIGNALS:
    void propertyChanged( KDPropertyInterface* );
public Q_SLOTS:
    void setFlat(bool);
    void update();
    void update(KDPropertyInterface*);

private:
    class Private;
    kdtools::pimpl_ptr<Private> d;
};

#endif /* __KDTOOLS_GUI_KDPROPERTYMODEL_H__ */

