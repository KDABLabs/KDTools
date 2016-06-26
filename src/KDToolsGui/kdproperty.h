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

#ifndef __KDTOOLS_GUI_KDPROPERTY_H__
#define __KDTOOLS_GUI_KDPROPERTY_H__

#include <KDToolsCore/kdtoolsglobal.h>

#include <QtCore/QDateTime>
#include <QtCore/QVariant>

#include <QtGui/QFont>

class KDPropertyModel;
class KDPropertyGroupInterface;

class KDTOOLSGUI_EXPORT KDPropertyInterface {
    KDPropertyInterface & operator=( const KDPropertyInterface & );
protected:
    KDPropertyInterface( const KDPropertyInterface & other );
public:
    KDPropertyInterface();
    virtual ~KDPropertyInterface();

    virtual KDPropertyInterface * clone() const = 0;

    void setParent( KDPropertyGroupInterface* );
    KDPropertyGroupInterface* parent() const;

    virtual void setModel( KDPropertyModel* );
    KDPropertyModel* model() const;

    void setChanged( bool );
    bool isChanged() const;

    void setHasReset(bool);
    bool hasReset() const;

    virtual void setEnabled( bool enable ) = 0;
    virtual bool isEnabled() const = 0;

    void setStandalone( bool standalone );
    bool isStandalone() const;

    virtual QString name() const = 0;

    virtual void setValue(const QVariant &value) = 0;
    virtual QVariant value() const = 0;

    void setHelpText( const QString& txt );
    QString helpText() const;

    virtual QString toString() const = 0;
    virtual QVariant decoration() const;

    virtual bool isContainer() const = 0;
    virtual bool isCategory() const = 0;

    virtual bool isEditable() const = 0;

    virtual QVariant editorType() const;
    virtual QVariant editorHint( int ) const;

    void update();

private:
    KDTOOLS_DECLARE_PRIVATE_BASE( KDPropertyInterface );
};

class KDTOOLSGUI_EXPORT KDPropertyGroupInterface : public KDPropertyInterface {
    KDPropertyGroupInterface & operator=( const KDPropertyGroupInterface & );
protected:
    KDPropertyGroupInterface( const KDPropertyGroupInterface & other );
public:
    KDPropertyGroupInterface();
    ~KDPropertyGroupInterface();

    virtual int indexOf(KDPropertyInterface* property) const = 0;
    virtual int propertyCount() const = 0;
    virtual KDPropertyInterface* propertyAt(int index) const = 0;

    void setModel( KDPropertyModel* ) KDAB_OVERRIDE;
    KDPropertyGroupInterface * clone() const KDAB_OVERRIDE = 0;
private:
    KDTOOLS_DECLARE_PRIVATE_DERIVED( KDPropertyGroupInterface, KDPropertyInterface );
};

class KDTOOLSGUI_EXPORT KDAbstractPropertyBase : public KDPropertyInterface {
    KDAbstractPropertyBase & operator=( const KDAbstractPropertyBase & );
protected:
    KDAbstractPropertyBase( const KDAbstractPropertyBase & other );
public:
    KDAbstractPropertyBase( const QVariant & v, const QString & name );
    ~KDAbstractPropertyBase();

    QVariant value() const KDAB_OVERRIDE;
    void setValue( const QVariant & value ) KDAB_OVERRIDE;

    QString name() const KDAB_OVERRIDE;
    bool isEditable() const KDAB_OVERRIDE;
    bool isContainer() const KDAB_OVERRIDE;
    bool isCategory() const KDAB_OVERRIDE;

    void setEnabled( bool e ) KDAB_OVERRIDE;
    bool isEnabled() const KDAB_OVERRIDE;

    KDAbstractPropertyBase * clone() const KDAB_OVERRIDE = 0;
private:
    KDTOOLS_DECLARE_PRIVATE_DERIVED( KDAbstractPropertyBase, KDPropertyInterface );
};

template <typename T>
class KDAbstractProperty : public KDAbstractPropertyBase {
    KDAbstractProperty & operator=( const KDAbstractProperty & );
protected:
    KDAbstractProperty( const KDAbstractProperty & other )
        : KDAbstractPropertyBase( other ) {}
public:
    explicit KDAbstractProperty( const QString & name_ )
        : KDAbstractPropertyBase( qVariantFromValue( T() ), name_ ) {}
    KDAbstractProperty( const T & value_, const QString & name_ )
        : KDAbstractPropertyBase( qVariantFromValue( value_ ), name_ ) {}

    T data() const { return qVariantValue<T>( this->value() ); }
    void setData( const T & data_ ) { this->setValue( data_ ); }

    KDAbstractProperty * clone() const KDAB_OVERRIDE = 0;
protected:
    KDAbstractProperty( KDAbstractPropertyBase::Private * _d_, bool b )
        : KDAbstractPropertyBase( _d_, b ) {}
};

class KDTOOLSGUI_EXPORT KDAbstractCompositeProperty : public KDPropertyGroupInterface {
    KDAbstractCompositeProperty & operator=( const KDAbstractCompositeProperty & other );
protected:
    KDAbstractCompositeProperty( const KDAbstractCompositeProperty & other );
public:
    explicit KDAbstractCompositeProperty( const QString & name );
    ~KDAbstractCompositeProperty();

    QString name() const KDAB_OVERRIDE;
    bool isContainer() const KDAB_OVERRIDE;
    bool isCategory() const KDAB_OVERRIDE;
    void setEnabled(bool e) KDAB_OVERRIDE;
    bool isEnabled() const KDAB_OVERRIDE;

    int indexOf( KDPropertyInterface * property ) const KDAB_OVERRIDE;
    int propertyCount() const KDAB_OVERRIDE;
    KDPropertyInterface* propertyAt(int index) const KDAB_OVERRIDE;

    KDAbstractCompositeProperty * clone() const KDAB_OVERRIDE = 0;

protected:
    void addProperty(KDPropertyInterface* property);

private:
    KDTOOLS_DECLARE_PRIVATE_DERIVED( KDAbstractCompositeProperty, KDPropertyGroupInterface );
};

class KDTOOLSGUI_EXPORT KDPropertyCategory : public KDPropertyGroupInterface {
    KDPropertyCategory & operator=( const KDPropertyCategory & other );
protected:
    KDPropertyCategory( const KDPropertyCategory & other );
public:
    explicit KDPropertyCategory( const QString & name, const QString & info=QString() );
    ~KDPropertyCategory();

    QString name() const KDAB_OVERRIDE;
    bool isContainer() const KDAB_OVERRIDE;
    bool isCategory() const KDAB_OVERRIDE;

    void setValue( const QVariant & ) KDAB_OVERRIDE;
    QVariant value() const KDAB_OVERRIDE;

    virtual void setIcon( const QIcon & icon );
    QIcon icon() const;

    QString toString() const KDAB_OVERRIDE;
    QVariant decoration() const KDAB_OVERRIDE;

    void setChecked( bool on );
    Qt::CheckState isChecked() const;

    void setCheckable(bool c);
    bool isCheckable() const;

    void setEnabled(bool e) KDAB_OVERRIDE;
    bool isEnabled() const KDAB_OVERRIDE;

    void addProperty(KDPropertyInterface* property);
    void removeProperty(KDPropertyInterface* property);

    int indexOf( KDPropertyInterface * property ) const KDAB_OVERRIDE;
    int propertyCount() const KDAB_OVERRIDE;
    KDPropertyInterface* propertyAt(int index) const KDAB_OVERRIDE;

    bool isEditable() const KDAB_OVERRIDE;

    KDPropertyCategory * clone() const KDAB_OVERRIDE;
private:
    KDTOOLS_DECLARE_PRIVATE_DERIVED( KDPropertyCategory, KDPropertyGroupInterface );
};

class KDTOOLSGUI_EXPORT KDStringProperty : public KDAbstractProperty<QString> {
    KDStringProperty & operator=( const KDStringProperty & other );
protected:
    KDStringProperty( const KDStringProperty & other );
public:
    KDStringProperty(const QString &value, const QString &name);
    KDStringProperty(const QString& mask, const QString &value, const QString &name);
    ~KDStringProperty();

    bool isPasswordMode() const;
    void setPasswordMode(bool m);

    QString inputMask() const;
    void setInputMask(const QString& );

    QString toString() const KDAB_OVERRIDE;

    QVariant editorType() const KDAB_OVERRIDE;
    QVariant editorHint(int) const KDAB_OVERRIDE;

    KDStringProperty * clone() const KDAB_OVERRIDE;
private:
    KDTOOLS_DECLARE_PRIVATE_DERIVED( KDStringProperty, KDAbstractProperty<QString> );
};


class KDTOOLSGUI_EXPORT KDComboProperty : public KDAbstractProperty<QString> {
    KDComboProperty & operator=( const KDComboProperty & other );
protected:
    KDComboProperty( const KDComboProperty & other );
public:
    KDComboProperty( const QStringList& choices, const QString& chosen, const QString& name );
    ~KDComboProperty();

    void setChoices( const QStringList& choices );
    QStringList choices() const;

    void setEditable( bool e );
    bool isEditable() const;

    QString toString() const KDAB_OVERRIDE;

    QVariant editorType() const KDAB_OVERRIDE;
    QVariant editorHint(int hint) const KDAB_OVERRIDE;

    KDComboProperty * clone() const KDAB_OVERRIDE;
private:
    KDTOOLS_DECLARE_PRIVATE_DERIVED( KDComboProperty, KDAbstractProperty<QString> );
};


class KDTOOLSGUI_EXPORT KDFilenameProperty : public KDAbstractProperty<QString> {
    KDFilenameProperty & operator=( const KDFilenameProperty & other );
protected:
    KDFilenameProperty( const KDFilenameProperty & other );
public:
    KDFilenameProperty(const QString &value, const QString &name);
    ~KDFilenameProperty();

    QString toString() const KDAB_OVERRIDE;
    QVariant editorType() const KDAB_OVERRIDE;

    KDFilenameProperty * clone() const KDAB_OVERRIDE;
private:
    KDTOOLS_DECLARE_PRIVATE_DERIVED( KDFilenameProperty, KDAbstractProperty<QString> );
};

class KDTOOLSGUI_EXPORT KDBooleanProperty : public KDAbstractProperty<bool> {
    KDBooleanProperty & operator=( const KDBooleanProperty & other );
protected:
    KDBooleanProperty( const KDBooleanProperty & other );
public:
    KDBooleanProperty( bool value, const QString & name, const QString & infotext=QString() );
    ~KDBooleanProperty();

    void setInfoText( const QString & info );
    QString infoText() const;

    QString toString() const KDAB_OVERRIDE;
    QVariant editorType() const KDAB_OVERRIDE;

    KDBooleanProperty * clone() const KDAB_OVERRIDE;
private:
    KDTOOLS_DECLARE_PRIVATE_DERIVED( KDBooleanProperty, KDAbstractProperty<bool> );
};

class KDTOOLSGUI_EXPORT KDIntProperty : public KDAbstractProperty<int> {
    KDIntProperty & operator=( const KDIntProperty & other );
protected:
    KDIntProperty( const KDIntProperty & other );
public:
    KDIntProperty(int value, const QString &name);
    KDIntProperty(int min, int max, int value, const QString &name);
    ~KDIntProperty();

    QString toString() const KDAB_OVERRIDE;
    QVariant editorHint(int hint) const KDAB_OVERRIDE;
    QVariant editorType() const KDAB_OVERRIDE;

    KDIntProperty * clone() const KDAB_OVERRIDE;
private:
    KDTOOLS_DECLARE_PRIVATE_DERIVED( KDIntProperty, KDAbstractProperty<int> );
};

class KDTOOLSGUI_EXPORT KDIntWithSliderProperty : public KDIntProperty {
    KDIntWithSliderProperty & operator=( const KDIntWithSliderProperty & other );
protected:
    KDIntWithSliderProperty( const KDIntWithSliderProperty & other );
public:
    KDIntWithSliderProperty(int value, const QString &name);
    KDIntWithSliderProperty(int min, int max, int value, const QString &name);
    ~KDIntWithSliderProperty();

    QVariant editorType() const KDAB_OVERRIDE;

    KDIntWithSliderProperty * clone() const KDAB_OVERRIDE;
private:
    KDTOOLS_DECLARE_PRIVATE_DERIVED( KDIntWithSliderProperty, KDIntProperty );
};

class KDTOOLSGUI_EXPORT KDUIntProperty : public KDAbstractProperty<uint> {
    KDUIntProperty & operator=( const KDUIntProperty & other );
protected:
    KDUIntProperty( const KDUIntProperty & other );
public:
    KDUIntProperty(uint value, const QString &name);
    ~KDUIntProperty();

    QString toString() const KDAB_OVERRIDE;

    KDUIntProperty * clone() const KDAB_OVERRIDE;
private:
    KDTOOLS_DECLARE_PRIVATE_DERIVED( KDUIntProperty, KDAbstractProperty<uint> );
};

class KDTOOLSGUI_EXPORT KDDoubleProperty : public KDAbstractProperty<double> {
    KDDoubleProperty & operator=( const KDDoubleProperty & other );
protected:
    KDDoubleProperty( const KDDoubleProperty & other );
public:
    KDDoubleProperty( double value, const QString & name );
    KDDoubleProperty( double min, double max, double value, const QString & name );
    ~KDDoubleProperty();

    QString toString() const KDAB_OVERRIDE;
    QVariant editorType() const KDAB_OVERRIDE;
    QVariant editorHint(int hint) const KDAB_OVERRIDE;

    KDDoubleProperty * clone() const KDAB_OVERRIDE;
private:
    KDTOOLS_DECLARE_PRIVATE_DERIVED( KDDoubleProperty, KDAbstractProperty<double> );
};

class KDTOOLSGUI_EXPORT KDDateProperty : public KDAbstractProperty<QDate> {
    KDDateProperty & operator=( const KDDateProperty & other );
protected:
    KDDateProperty( const KDDateProperty & other );
public:
    KDDateProperty(const QDate& value, const QString &name);
    ~KDDateProperty();

    QString toString() const KDAB_OVERRIDE;

    KDDateProperty * clone() const KDAB_OVERRIDE;
private:
    KDTOOLS_DECLARE_PRIVATE_DERIVED( KDDateProperty, KDAbstractProperty<QDate> );
};


class KDTOOLSGUI_EXPORT KDDateTimeProperty : public KDAbstractProperty<QDateTime> {
    KDDateTimeProperty & operator=( const KDDateTimeProperty & other );
protected:
    KDDateTimeProperty( const KDDateTimeProperty & other );
public:
    KDDateTimeProperty(const QDateTime& value, const QString &name);
    ~KDDateTimeProperty();

    QString toString() const KDAB_OVERRIDE;

    KDDateTimeProperty * clone() const KDAB_OVERRIDE;
private:
    KDTOOLS_DECLARE_PRIVATE_DERIVED( KDDateTimeProperty, KDAbstractProperty<QDateTime> );
};


class KDTOOLSGUI_EXPORT KDChoiceProperty : public KDAbstractProperty<int> {
    KDChoiceProperty & operator=( const KDChoiceProperty & other );
protected:
    KDChoiceProperty( const KDChoiceProperty & other );
public:
    KDChoiceProperty( const QStringList& choices, int chosen, const QString& name );
    ~KDChoiceProperty();

    void setChoices( const QStringList& choices );
    QStringList choices() const;

    QString toString() const KDAB_OVERRIDE;

    QVariant editorType() const KDAB_OVERRIDE;
    QVariant editorHint(int hint) const KDAB_OVERRIDE;

    KDChoiceProperty * clone() const KDAB_OVERRIDE;
private:
    KDTOOLS_DECLARE_PRIVATE_DERIVED( KDChoiceProperty, KDAbstractProperty<int> );
};

class KDTOOLSGUI_EXPORT KDMultipleChoiceProperty : public KDAbstractProperty<QList<QVariant> > {
    KDMultipleChoiceProperty & operator=( const KDMultipleChoiceProperty & other );
protected:
    KDMultipleChoiceProperty( const KDMultipleChoiceProperty & other );
public:
    KDMultipleChoiceProperty( const QStringList& choices, QList<QVariant> values, const QString& name );
    ~KDMultipleChoiceProperty();

    void setChoices( const QStringList& choices );
    QStringList choices() const;

    QString toString() const KDAB_OVERRIDE;

    QVariant editorType() const KDAB_OVERRIDE;
    QVariant editorHint(int hint) const KDAB_OVERRIDE;

    KDMultipleChoiceProperty * clone() const KDAB_OVERRIDE;
private:
    KDTOOLS_DECLARE_PRIVATE_DERIVED( KDMultipleChoiceProperty, KDAbstractProperty<QList<QVariant> > );
};

class KDTOOLSGUI_EXPORT KDColorProperty : public KDAbstractProperty<QColor> {
    KDColorProperty & operator=( const KDColorProperty & other );
protected:
    KDColorProperty( const KDColorProperty & other );
public:
    KDColorProperty(const QColor& value, const QString &name);
    ~KDColorProperty();

    QString toString() const KDAB_OVERRIDE;
    QVariant decoration() const KDAB_OVERRIDE;

    QVariant editorType() const KDAB_OVERRIDE;

    KDColorProperty * clone() const KDAB_OVERRIDE;
private:
    KDTOOLS_DECLARE_PRIVATE_DERIVED( KDColorProperty, KDAbstractProperty<QColor> );
};

class KDTOOLSGUI_EXPORT KDFontProperty : public KDAbstractProperty<QFont> {
    KDFontProperty & operator=( const KDFontProperty & other );
protected:
    KDFontProperty( const KDFontProperty & other );
public:
    KDFontProperty(const QFont& value, const QString &name);
    ~KDFontProperty();

    QString toString() const KDAB_OVERRIDE;
    QVariant decoration() const KDAB_OVERRIDE;

    QVariant editorType() const KDAB_OVERRIDE;

    KDFontProperty * clone() const KDAB_OVERRIDE;
private:
    KDTOOLS_DECLARE_PRIVATE_DERIVED( KDFontProperty, KDAbstractProperty<QFont> );
};

class KDTOOLSGUI_EXPORT KDSizeProperty : public KDAbstractCompositeProperty {
    KDSizeProperty & operator=( const KDSizeProperty & other );
protected:
    KDSizeProperty( const KDSizeProperty & other );
public:
    KDSizeProperty( const QSize& value, const QString& name );
    ~KDSizeProperty();

    QVariant value() const KDAB_OVERRIDE;
    void setValue( const QVariant& ) KDAB_OVERRIDE;

    QString toString() const KDAB_OVERRIDE;

    bool isEditable() const KDAB_OVERRIDE;

    KDSizeProperty * clone() const KDAB_OVERRIDE;
private:
    KDTOOLS_DECLARE_PRIVATE_DERIVED( KDSizeProperty, KDAbstractCompositeProperty );
};

#endif /* __KDTOOLS_GUI_KDPROPERTY_H__ */

