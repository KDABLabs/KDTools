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

#ifndef __KDTOOLS__GUI__KDDOUBLESPINBOX_H__
#define __KDTOOLS__GUI__KDDOUBLESPINBOX_H__

#include <KDToolsCore/pimpl_ptr>

#if QT_VERSION < 0x050000
#include <QtGui/QDoubleSpinBox>
#else
#include <QtWidgets/QDoubleSpinBox>
#endif

class KDTOOLSGUI_EXPORT KDDoubleSpinBox : public QDoubleSpinBox
{
    Q_OBJECT
    Q_PROPERTY( bool showGroupSeparators READ showGroupSeparators WRITE setShowGroupSeparators )
    Q_CLASSINFO( "description", "A double spinbox with improved locale support" )
public:
    explicit KDDoubleSpinBox( QWidget* parent = 0 );
    ~KDDoubleSpinBox();

    void setShowGroupSeparators( bool show );
    bool showGroupSeparators() const;

    QString textFromValue( double value ) const KDAB_OVERRIDE;
    double valueFromText( const QString & text ) const KDAB_OVERRIDE;

    QValidator::State validate( QString & text, int & pos ) const KDAB_OVERRIDE;

    bool event( QEvent * ) KDAB_OVERRIDE;

private:
    class Private;
    kdtools::pimpl_ptr< Private > d;
};

class KDTOOLSGUI_EXPORT KDSpinBox : public QSpinBox
{
    Q_OBJECT
    Q_PROPERTY( bool showGroupSeparators READ showGroupSeparators WRITE setShowGroupSeparators )
    Q_CLASSINFO( "description", "A spinbox with improved locale support" )
public:
    explicit KDSpinBox( QWidget* parent = 0 );
    ~KDSpinBox();

    void setShowGroupSeparators( bool show );
    bool showGroupSeparators() const;

    QString textFromValue( int value ) const KDAB_OVERRIDE;
    int valueFromText( const QString & text ) const KDAB_OVERRIDE;

    QValidator::State validate( QString & text, int & pos ) const KDAB_OVERRIDE;

    bool event( QEvent * ) KDAB_OVERRIDE;

private:
    class Private;
    kdtools::pimpl_ptr< Private > d;
};

#endif /* __KDTOOLS__GUI__KDDOUBLESPINBOX_H__ */
