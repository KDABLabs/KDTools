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

#ifndef __KDTOOLS_GUI_KDEMAILVALIDATOR_H__
#define __KDTOOLS_GUI_KDEMAILVALIDATOR_H__

#include <QtGui/QValidator>

#include <KDToolsCore/kdtoolsglobal.h>
#include <KDToolsCore/pimpl_ptr.h>

#if QT_VERSION >= 0x040200 && !defined( KDTOOLS_SUPPORTIDNA )
#define KDTOOLS_SUPPORTIDNA
#endif

class KDTOOLSGUI_EXPORT KDEmailValidator : public QValidator {
    Q_OBJECT
    Q_ENUMS( Mode )
#ifdef KDTOOLS_SUPPORTIDNA
    Q_PROPERTY( bool allowInternationalizedDomainNames READ allowInternationalizedDomainNames WRITE setAllowInternationalizedDomainNames )
    Q_PROPERTY( QString idnEvaluationDomain READ idnEvaluationDomain WRITE setIdnEvaluationDomain )
#endif
    Q_PROPERTY( Mode mode READ mode WRITE setMode )
    Q_CLASSINFO( "description", "A QValidator to validate email addresses" )
public:
    enum Mode {
        LocalPart,
        Domain,
        AddrSpec,

        _NumModes
    };

    explicit KDEmailValidator( QObject * parent=0 );
    explicit KDEmailValidator( Mode mode, QObject * parent=0 );
    ~KDEmailValidator();

    Mode mode() const;
#ifdef KDTOOLS_SUPPORTIDNA
    bool allowInternationalizedDomainNames() const;
    QString idnEvaluationDomain() const;
#endif

    State validate( QString & s, int & p ) const KDAB_OVERRIDE;

public Q_SLOTS:
    void setMode( Mode mode );
#ifdef KDTOOLS_SUPPORTIDNA
    void setAllowInternationalizedDomainNames( bool allow );
    void setIdnEvaluationDomain( const QString & tld );
#endif

private:
    class Private;
    kdtools::pimpl_ptr<Private> d;
};

#endif /* __KDTOOLS_GUI_KDEMAILVALIDATOR_H__ */
