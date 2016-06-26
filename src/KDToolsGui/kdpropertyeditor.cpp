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

#include "kdpropertyeditor.h"
#include "kdpropertymodel.h"
#include "kdproperty.h"

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QColor>
#include <QColorDialog>
#include <QFileDialog>
#include <QFontDialog>
#include <QHash>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QSpinBox>
#include <QToolBar>
#include <QToolButton>

/*!\class KDPropertyEditorFactory KDPropertyEditor
  \ingroup KDPropertyView
  \brief A factory class that can created named property
  editors as used by KDPropertyViewDelegate.

  Normally you don't need to instantiate this yourself,
  just use the instance provided by
  KDPropertyEditorFactory::defaultFactory()
*/

KDPropertyEditor::~KDPropertyEditor()
{
}

class KDPropertyEditorFactory::Private {
    friend class ::KDPropertyEditorFactory;
public:
    Private() {}
    ~Private() {}

private:
    QHash<QString,KDPropertyEditor*> editors;
};

/*! Returns a pointer to the default KDPropertyEditorFactory
  used by the application.

  \todo Does it make sense to allow the user to specify
  a non-default propertyeditorfactory?
*/
KDPropertyEditorFactory* KDPropertyEditorFactory::defaultFactory()
{
  static KDPropertyEditorFactory instance;
  return &instance;
}

KDPropertyEditorFactory::KDPropertyEditorFactory()
    : d( new Private )
{

}

KDPropertyEditorFactory::~KDPropertyEditorFactory()
{
  qDeleteAll(d->editors);
}

/*! Add a new property editor type to the factory. If an editor with
  this type already exists, it will be replaced.

  Normally, you won't call this directly. Use the
  REGISTER_KDPROPERTYEDITOR( className ) macro instead.
*/
void KDPropertyEditorFactory::addPropertyEditorType( const QString& type, KDPropertyEditor* editor )
{
  QHash<QString,KDPropertyEditor*>::iterator it = d->editors.find(type);
  if( it != d->editors.end() ) {
    delete *it;
    d->editors.erase(it);
  }
  d->editors.insert(type,editor);
}

/*! Returns a pointer to an property editor with type \a type.
  For maximum flexibility the type is the classname of the editor.

  By default types "KDStringPropertyEditor", "KDComboPropertyEditor", "KDIntPropertyEditor"
  "KDIntSliderPropertyEditor", "KDBooleanPropertyEditor", "KDDoublePropertyEditor"
  "KDChoicePropertyEditor",  "KDMultipleChoicePropertyEditor" and "KDColorPropertyEditor"
  are provided plus the default editor provided by Qt in case no custom
  editor is specified. \see KDProperty::editorType() const

  \todo Determine which standard editor types we need to supply.
*/
KDPropertyEditor* KDPropertyEditorFactory::editor( const QString& type )
{
  QHash<QString,KDPropertyEditor*>::iterator it = d->editors.find(type);
  if( it != d->editors.end() ) {
    return *it;
  } else {
    return 0;
  }
}

/*!\class KDPropertyEditor KDPropertyEditor
  \ingroup KDPropertyView
  \brief Abstract baseclass for all special property editors.

  Subclass, implement the pure virtual methods and register
  it with the REGISTER_KDPROPERTYEDITOR() macro.
*/

/*!\fn KDPropertyEditor::createEditor(QWidget* parent, const QObject* target, const char* receiver)
  Implement this to return an instance of an editor widget with parent
  \a parent. The widget must emit a signal when the value in it is changed.
  This signal must be connected to the slot \a receiver in the object
  \a target.
 */

/*!\fn KDPropertyEditor::setEditorData(QWidget *editor, const QModelIndex& index )
  Implement this to copy the value from the data item pointed to
  by \a index to the editor widget \a editor.
*/

/*!\fn KDPropertyEditor::setModelData(QWidget *editor, QAbstractItemModel* model,const QModelIndex& index )
  Implement this to copy the edited value from the editor widget to the
  date item in \a model pointed to by \a index.
*/

/*!\def REGISTER_KDPROPERTYEDITOR( className )
  \ingroup KDPropertyView
  When implementing KDPropertyEditor subclasses, use
  this macro after the implementation to register the
  editor with the factory.
*/

class KDStringPropertyEditor : public KDPropertyEditor {
public:
  virtual QWidget *createEditor(QWidget *parent, const QObject *target, const char *receiver)
  {
    QLineEdit* ed = new QLineEdit(parent);
    ed->setFrame(false);
    QObject::connect( ed, SIGNAL(returnPressed()),
		      target, receiver);
    return ed;
  }

  virtual void setEditorData(QWidget *editor, const QModelIndex& index )
  {
    if(QLineEdit *lineEdit = qobject_cast<QLineEdit*>(editor)) {
      QString str = index.model()->data( index, Qt::EditRole ).toString();
      if(lineEdit->text() != str )
	lineEdit->setText(str);
      QString mask = index.model()->data( index, KDPropertyModel::InputMaskHint ).toString();
      if( !mask.isEmpty() ) lineEdit->setInputMask(mask);
      if( index.model()->data( index, KDPropertyModel::PasswordModeHint ).toBool() ) {
	lineEdit->setEchoMode(QLineEdit::Password);
      } else {
	lineEdit->setEchoMode(QLineEdit::Normal);
      }
    }
  }

  virtual void setModelData(QWidget *editor, QAbstractItemModel* model, const QModelIndex& index )
  {
    if(QLineEdit *lineEdit = qobject_cast<QLineEdit*>(editor)) {
      QString newValue = lineEdit->text();
      QString oldValue = model->data( index, Qt::EditRole ).toString();
      if (newValue != oldValue ) {
	model->setData( index, newValue, Qt::EditRole );
      }
    }
  }
};

class KDBooleanPropertyEditor : public KDPropertyEditor {
public:
  virtual QWidget *createEditor(QWidget *parent, const QObject *target, const char *receiver)
  {
    QCheckBox* cb = new QCheckBox(parent);
    QObject::connect( cb, SIGNAL(clicked()),
		      target, receiver);
    return cb;
  }

  virtual void setEditorData(QWidget *editor, const QModelIndex& index )
  {
    if(QCheckBox *cb = qobject_cast<QCheckBox*>(editor)) {
      bool c = index.model()->data( index, Qt::EditRole ).toBool();
      if(cb->isChecked() != c )
	cb->setChecked(c);
    }
  }

  virtual void setModelData(QWidget *editor, QAbstractItemModel* model, const QModelIndex& index )
  {
    if(QCheckBox *cb = qobject_cast<QCheckBox*>(editor)) {
      bool newValue = cb->isChecked();
      bool oldValue = model->data( index, Qt::EditRole ).toBool();
      if (newValue != oldValue ) {
	model->setData( index, newValue, Qt::EditRole );
      }
    }
  }
};

class KDComboPropertyWidget : public QWidget {
  Q_OBJECT
public:
  Q_PROPERTY( QString text READ text WRITE setChosen )

  KDComboPropertyWidget(QWidget* parent = 0) : QWidget(parent)
  {
    m_combobox = new QComboBox;
    QHBoxLayout* l = new QHBoxLayout(this);
    l->setMargin(0);
    l->setSpacing(0);
    l->addWidget(m_combobox);
    connect(m_combobox,SIGNAL(currentIndexChanged(QString)),
	    this, SIGNAL(valueChanged(QString)));
  }

  void setChoices( const QStringList& choices )
  {
    m_choices = choices;
    m_combobox->clear();
    m_combobox->addItems( choices );
  }

  const QStringList& choices() const { return m_choices; }

  void setEditable(bool e) { m_combobox->setEditable(e); }

  void setChosen( const QString& txt)
  {
    int idx = m_combobox->findText(txt);
    if( idx>=0 ) m_combobox->setCurrentIndex( idx );
    else m_combobox->setEditText(txt);
  }
  QString chosen() const
  {
    return m_combobox->currentText();
  }
  QString text() const { return chosen(); }

Q_SIGNALS:
   void valueChanged(const QString& value);
private:
   QStringList m_choices;
   QComboBox* m_combobox;
};

class KDComboPropertyEditor : public KDPropertyEditor {
public:
   virtual QWidget *createEditor(QWidget *parent, const QObject *target, const char *receiver)
   {
      KDComboPropertyWidget* ed = new KDComboPropertyWidget(parent);
      QObject::connect( ed, SIGNAL(valueChanged(QString)),
         target, receiver);
      return ed;
   }

   virtual void setEditorData(QWidget *editor, const QModelIndex& index )
   {
     if (KDComboPropertyWidget* ed = qobject_cast<KDComboPropertyWidget*>
	 (editor)) {
       QStringList choices = index.model()->data( index,
						  KDPropertyModel::ChoiceOptionsHint ).toStringList();
       if( ed->choices() != choices ) ed->setChoices(choices);
       ed->setEditable( index.model()->data(index,KDPropertyModel::EditableHint).toBool());

       QString value = qVariantValue<QString>(index.model()->data( index, Qt::EditRole ));
       if( value != ed->chosen())
	 ed->setChosen( value );

     }
   }

  virtual void setModelData(QWidget *editor, QAbstractItemModel* model, const QModelIndex& index )
  {
    Q_ASSERT(editor);
    Q_ASSERT(model);
    if(KDComboPropertyWidget* ed = qobject_cast<KDComboPropertyWidget*>
       (editor)) {
      QString newvalue = ed->chosen();
      QString oldvalue = qVariantValue<QString>(model->data( index, Qt::EditRole ));

      if (newvalue != oldvalue ) {
	model->setData( index, qVariantFromValue(newvalue), Qt::EditRole );
      }
    }
  }
};


template <typename Editor, typename T>
class KDDialogPropertyEditor : public KDPropertyEditor {
public:
  KDDialogPropertyEditor( const char* sn ) : signalName(sn) {}
  virtual QWidget *createEditor(QWidget *parent, const QObject *target, const char *receiver)
  {
    Editor* ed = new Editor(parent);
    QObject::connect( ed, signalName,
          target, receiver);
    return ed;
  }

  virtual void setEditorData(QWidget *editor, const QModelIndex& index )
  {
    if( Editor* ed = qobject_cast<Editor*>(editor)) {
      T data = qVariantValue<T>(index.model()->data( index, Qt::EditRole ));
      if(editorValue(ed) != data )
  setEditorValue(ed,data);
    }
  }

  virtual void setModelData(QWidget *editor, QAbstractItemModel* model, const QModelIndex& index )
  {
    if( Editor* ed = qobject_cast<Editor*>(editor)) {
      T newValue = editorValue(ed);
      T oldValue = qVariantValue<T>(model->data( index, Qt::EditRole ));
      if (newValue != oldValue ) {
  model->setData( index, qVariantFromValue(newValue), Qt::EditRole );
      }
    }
  }
protected:
  virtual T editorValue(Editor*) = 0;
  virtual void setEditorValue(Editor*,T) = 0;

  const char* signalName;
};

class KDDoublePropertyEditor : public KDPropertyEditor {
public:
  virtual QWidget *createEditor(QWidget *parent, const QObject *target, const char *receiver)
  {
    QDoubleSpinBox* spinBox = new QDoubleSpinBox(parent);
    QObject::connect (spinBox, SIGNAL(editingFinished()), target, receiver);
    return spinBox;
  }

  virtual void setEditorData(QWidget *editor, const QModelIndex &index)
  {
    if ( QDoubleSpinBox* spinBox = qobject_cast<QDoubleSpinBox*>(editor) ) {
      if ( index.model()->data( index, KDPropertyModel::MinimumHint ).isValid() ) {
        spinBox->setMinimum(index.model()->data( index, KDPropertyModel::MinimumHint ).toDouble());
      }
      if ( index.model()->data( index, KDPropertyModel::MaximumHint ).isValid() ) {
        spinBox->setMaximum(index.model()->data( index, KDPropertyModel::MaximumHint ).toDouble());
      }

      const double val = index.model()->data( index, Qt::EditRole ).toInt();
      if (spinBox->value() != val ) {
        spinBox->setValue(val);
      }
    }
  }

  virtual void setModelData(QWidget *editor, QAbstractItemModel* model, const QModelIndex& index )
  {
    if ( const QDoubleSpinBox* spinBox = qobject_cast<QDoubleSpinBox*>(editor) ) {
      const double newValue = spinBox->value();
      const double oldValue = model->data( index, Qt::EditRole ).toDouble();
      if (newValue != oldValue ) {
        model->setData( index, newValue, Qt::EditRole );
      }
    }
  }
};

#ifndef QT_NO_FILEDIALOG
/* internal */
class KDFilenameEditor : public QWidget {
  Q_OBJECT
public:
  explicit KDFilenameEditor( QWidget* parent = 0 )
    : QWidget(parent)
  {
    setFocusPolicy( Qt::StrongFocus );
    m_lineedit = new QLineEdit;
    m_lineedit->setFrame(false);
    m_button = new QToolButton;
    m_button->setIcon( m_button->style()->standardPixmap(QStyle::SP_DirOpenIcon) );

    QHBoxLayout* l = new QHBoxLayout(this);
    l->setMargin(1);
    l->setSpacing(1);
    l->addWidget(m_lineedit);
    l->addWidget(m_button);

    connect( m_lineedit, SIGNAL(textChanged(QString)),
	     this, SIGNAL(filenameChanged(QString)));
    connect( m_button, SIGNAL(clicked()),
	     this, SLOT(slotButtonClicked()));
  }
  QString filename() const { return m_lineedit->text(); }
  void setFilename( const QString& fname ) { m_lineedit->setText(fname); }

Q_SIGNALS:
  void filenameChanged(const QString&);
public Q_SLOTS:
  void slotButtonClicked()
  {
    QString fname = QFileDialog::getOpenFileName( this, m_lineedit->text() );
    if( !fname.isEmpty() ) m_lineedit->setText(fname);
  }
private:
  QLineEdit* m_lineedit;
  QAbstractButton* m_button;
};

class KDFilenamePropertyEditor :
  public KDDialogPropertyEditor<KDFilenameEditor,QString> {
public:
  KDFilenamePropertyEditor()
    : KDDialogPropertyEditor<KDFilenameEditor,QString>(SIGNAL(filenameChanged(QString))) {}
protected:
  virtual QString editorValue(KDFilenameEditor* ed) { return ed->filename(); }
  virtual void setEditorValue(KDFilenameEditor* ed, QString data) { ed->setFilename(data); }
};
#endif // QT_NO_FILEDIALOG

class KDIntPropertyEditor : public KDPropertyEditor {
public:
  virtual QWidget *createEditor(QWidget *parent, const QObject *target, const char *receiver)
  {
    QSpinBox* spinBox = new QSpinBox(parent);
    QObject::connect (spinBox, SIGNAL(editingFinished()), target, receiver);
    return spinBox;
  }

  virtual void setEditorData(QWidget *editor, const QModelIndex &index)
  {
    if ( QSpinBox* spinBox = qobject_cast<QSpinBox*>(editor) ) {
      if ( index.model()->data( index, KDPropertyModel::MinimumHint ).isValid() ) {
        spinBox->setMinimum(index.model()->data( index, KDPropertyModel::MinimumHint ).toInt());
      }
      if ( index.model()->data( index, KDPropertyModel::MaximumHint ).isValid() ) {
        spinBox->setMaximum(index.model()->data( index, KDPropertyModel::MaximumHint ).toInt());
      }

      const int val = index.model()->data( index, Qt::EditRole ).toInt();
      if (spinBox->value() != val ) {
        spinBox->setValue(val);
      }
    }
  }

  virtual void setModelData(QWidget *editor, QAbstractItemModel* model, const QModelIndex& index )
  {
    if ( const QSpinBox* spinBox = qobject_cast<QSpinBox*>(editor) ) {
      const int newValue = spinBox->value();
      const int oldValue = model->data( index, Qt::EditRole ).toInt();
      if (newValue != oldValue ) {
        model->setData( index, newValue, Qt::EditRole );
      }
    }
  }
};

/* internal */
class LabeledSlider : public QWidget {
  Q_OBJECT
public:
  explicit LabeledSlider( QWidget* parent=0 ) : QWidget(parent)
  {
    setFocusPolicy( Qt::StrongFocus );
    setAutoFillBackground(true);
    QHBoxLayout* l = new QHBoxLayout(this);
    l->setSpacing(2);
    l->setMargin(1);
    QLabel* label = new QLabel;
    m_slider = new QSlider;
    m_slider->setOrientation( Qt::Horizontal );
    l->addWidget(label);
    l->addWidget(m_slider);
    connect( m_slider, SIGNAL(valueChanged(int)),
	     label, SLOT(setNum(int)));
  }
  QSlider* slider() { return m_slider; }
private:
  QSlider* m_slider;
};

class KDIntSliderPropertyEditor : public KDPropertyEditor {
public:
  virtual QWidget *createEditor(QWidget *parent, const QObject *target, const char *receiver)
  {
    LabeledSlider* ed = new LabeledSlider(parent);
    QObject::connect( ed->slider(), SIGNAL(valueChanged(int)),
		      target, receiver);
    return ed;
  }

  virtual void setEditorData(QWidget *editor, const QModelIndex& index )
  {
    if(LabeledSlider* slider = qobject_cast<LabeledSlider*>(editor)) {
      int val = index.model()->data( index, Qt::EditRole ).toInt();
      int min = 0;
      int max = 100;
      if( index.model()->data( index, KDPropertyModel::MinimumHint ) != QVariant() ) {
	min = index.model()->data( index, KDPropertyModel::MinimumHint ).toInt();
      }
      if( index.model()->data( index, KDPropertyModel::MaximumHint ) != QVariant() ) {
	max = index.model()->data( index, KDPropertyModel::MaximumHint ).toInt();
      }
      slider->slider()->setRange(min,max);
      if(slider->slider()->value() != val ) {
	slider->slider()->setValue(val);
      }
    }
  }

  virtual void setModelData(QWidget *editor, QAbstractItemModel* model, const QModelIndex& index )
  {
    if(LabeledSlider* slider = qobject_cast<LabeledSlider*>(editor)) {
      int newValue = slider->slider()->value();
      int oldValue = model->data( index, Qt::EditRole ).toInt();
      if (newValue != oldValue ) {
	model->setData( index, newValue, Qt::EditRole );
	//model->setData( index, true, KDProperty::ChangedRole );
      }
    }
  }
};


#ifndef QT_NO_COLORDIALOG
class KDColorPropertyEditorWidget : public QWidget {
  Q_OBJECT
public:
  explicit KDColorPropertyEditorWidget( QWidget* parent = 0 )
    : QWidget(parent)
  {
    setFocusPolicy( Qt::StrongFocus );
    QToolButton* tb = new QToolButton( this );
    tb->setIcon( style()->standardPixmap( QStyle::SP_DesktopIcon ) );
    connect( tb, SIGNAL(clicked()),
	     this, SLOT(slotEditColor()));

    QHBoxLayout* l = new QHBoxLayout(this);
    l->setMargin(1);
    l->setSpacing(0);
    l->addStretch();
    l->addWidget(tb);
  }

  QColor color() const { return m_color; }
  void setColor( const QColor& color )
  {
    m_color = color;

    // TODO(steffen): Find a way to get dialog up
    // quickly
  }

Q_SIGNALS:
  void colorChanged(const QColor&);
private Q_SLOTS:
  void slotEditColor()
  {
      QColor c = QColorDialog::getColor( m_color, this );
      if( c.isValid() && c != m_color ) {
      m_color = c;
      emit colorChanged(m_color);
    }
  }
private:
  QColor m_color;
};

class KDColorPropertyEditor : public KDDialogPropertyEditor<KDColorPropertyEditorWidget,QColor> {
public:
  KDColorPropertyEditor()
    : KDDialogPropertyEditor<KDColorPropertyEditorWidget,QColor>(SIGNAL(colorChanged(QColor))) {}
protected:
  virtual QColor editorValue(KDColorPropertyEditorWidget* ed) { return ed->color(); }
  virtual void setEditorValue(KDColorPropertyEditorWidget* ed, QColor data) { ed->setColor(data); }
};
#endif // QT_NO_COLORDIALOG

class KDChoicePropertyWidget : public QWidget {
  Q_OBJECT
public:
  KDChoicePropertyWidget(QWidget* parent = 0) : QWidget(parent)
  {
    QVBoxLayout* l = new QVBoxLayout(this);
    l->setMargin(1);
    l->setSpacing(0);
    setLayout(l);
    m_grp = new QButtonGroup(this);
    connect(m_grp,SIGNAL(buttonClicked(int)),
        this,SIGNAL(buttonClicked(int)));
  }
  void setChoices( const QStringList& choices )
  {
    m_choices = choices;
    qDeleteAll(m_buttons.begin(),m_buttons.end());
    m_buttons.clear();
    int i = 0;
    Q_FOREACH( const QString &c, choices ) {
      QRadioButton* rb = new QRadioButton( c );
      rb->setAutoFillBackground(true);
      m_grp->addButton(rb);
      m_grp->setId(rb,i++);
      layout()->addWidget(rb);
      m_buttons.push_back(rb);
      rb->show();
    }
  }
  const QStringList& choices() const { return m_choices; }

  void setChosen(int i)
  {
    Q_ASSERT(i>=0 && i<m_buttons.size());
    m_buttons[i]->setChecked(true);
  }

  int chosen() const { return m_grp->checkedId(); }

Q_SIGNALS:
  void buttonClicked(int);
private:
  QButtonGroup* m_grp;
  QList<QRadioButton*> m_buttons;
  QStringList m_choices;
};

class KDMultipleChoicePropertyWidget : public QWidget {
  Q_OBJECT
public:
  KDMultipleChoicePropertyWidget(QWidget* parent = 0) : QWidget(parent)
  {
    QVBoxLayout* l = new QVBoxLayout(this);
    l->setMargin(1);
    l->setSpacing(0);
    setLayout(l);
    m_grp = new QButtonGroup(this);
    m_grp->setExclusive(false);
    connect(m_grp,SIGNAL(buttonClicked(int)),
        this,SIGNAL(buttonClicked(int)));
  }

  void setChoices( const QStringList& choices )
  {
    m_choices = choices;
    qDeleteAll(m_buttons.begin(),m_buttons.end());
    m_buttons.clear();
    int i = 0;
    Q_FOREACH( const QString &c, choices ) {
      QCheckBox* cb = new QCheckBox( c );
      cb->setAutoFillBackground(true);
      m_grp->addButton(cb);
      m_grp->setId(cb,i++);
      layout()->addWidget(cb);
      m_buttons.push_back(cb);
      cb->show();
    }
  }
  const QStringList& choices() const { return m_choices; }

  void setValues(QList<QVariant> values)
  {
      Q_FOREACH( const QVariant &v, values ) {
          int i = v.toInt();
          Q_ASSERT(i>=0 && i<m_buttons.size());
          m_buttons[i]->setChecked(true);
      }
  }

  QList<QVariant> values() const {
      QList<QVariant> values;
      for( int i = 0; i < m_buttons.size(); i++) {
          if(m_buttons.at(i)->isChecked())
              values.append(i);
      }
      return values;
  }

Q_SIGNALS:
  void buttonClicked(int);
private:
  QButtonGroup* m_grp;
  QList<QCheckBox*> m_buttons;
  QStringList m_choices;
};


#ifndef QT_NO_FONTDIALOG
class KDFontPropertyEditorWidget : public QWidget {
  Q_OBJECT
public:
  explicit KDFontPropertyEditorWidget( QWidget* parent = 0 )
    : QWidget(parent)
  {
    setFocusPolicy( Qt::StrongFocus );
    QToolButton* tb = new QToolButton;
    tb->setIcon( style()->standardPixmap( QStyle::SP_DesktopIcon ) );
    connect( tb, SIGNAL(clicked()),
	     this, SLOT(slotEditFont()));

    QHBoxLayout* l = new QHBoxLayout(this);
    l->setMargin(1);
    l->setSpacing(0);
    l->addStretch();
    l->addWidget(tb);
  }

  QFont font() const { return m_font; }
  void setFont( const QFont& font )
  {
    m_font = font;
    // TODO(steffen): Find a way to get dialog up
    // quickly
  }

Q_SIGNALS:
  void fontChanged(const QFont&);
private Q_SLOTS:
  void slotEditFont()
  {
    bool ok;
    QFont f = QFontDialog::getFont( &ok, m_font, this );
    if( ok && f != m_font ) {
      m_font = f;
      emit fontChanged(m_font);
    }
  }
private:
  QFont m_font;
};

class KDFontPropertyEditor : public KDDialogPropertyEditor<KDFontPropertyEditorWidget,QFont> {
public:
  KDFontPropertyEditor()
    : KDDialogPropertyEditor<KDFontPropertyEditorWidget,QFont>(SIGNAL(fontChanged(QFont))) {}
protected:
  virtual QFont editorValue(KDFontPropertyEditorWidget* ed) { return ed->font(); }
  virtual void setEditorValue(KDFontPropertyEditorWidget* ed, QFont data) { ed->setFont(data); }
};
#endif // QT_NO_FONTDIALOG


class KDChoicePropertyEditor : public KDPropertyEditor {
public:
  virtual QWidget *createEditor(QWidget *parent, const QObject *target, const char *receiver)
  {
    KDChoicePropertyWidget* ed = new KDChoicePropertyWidget(parent);
    QObject::connect( ed, SIGNAL(buttonClicked(int)),
              target, receiver);
    return ed;
  }

  virtual void setEditorData(QWidget *editor, const QModelIndex& index )
  {
    if(KDChoicePropertyWidget* ed = qobject_cast<KDChoicePropertyWidget*>(editor)) {
      QStringList choices = index.model()->data( index, KDPropertyModel::ChoiceOptionsHint ).toStringList();
      if( ed->choices() != choices ) ed->setChoices(choices);
      int i = qVariantValue<int>(index.model()->data( index, Qt::EditRole ));
      if( i != ed->chosen())
    ed->setChosen(i);
    }
  }

  virtual void setModelData(QWidget *editor, QAbstractItemModel* model, const QModelIndex& index )
  {
    if(KDChoicePropertyWidget* ed = qobject_cast<KDChoicePropertyWidget*>(editor)) {
      int newc = ed->chosen();
      int oldc = qVariantValue<int>(model->data( index, Qt::EditRole ));
      if (newc != oldc ) {
    model->setData( index, qVariantFromValue(newc), Qt::EditRole );
      }
    }
  }
};

class KDMultipleChoicePropertyEditor : public KDPropertyEditor {
public:
  virtual QWidget *createEditor(QWidget *parent, const QObject *target, const char *receiver)
  {
    KDMultipleChoicePropertyWidget* ed = new KDMultipleChoicePropertyWidget(parent);
    QObject::connect( ed, SIGNAL(buttonClicked(int)),
              target, receiver);
    return ed;
  }

  virtual void setEditorData(QWidget *editor, const QModelIndex& index )
  {
    if(KDMultipleChoicePropertyWidget* ed = qobject_cast<KDMultipleChoicePropertyWidget*>(editor)) {
      QStringList choices = index.model()->data( index, KDPropertyModel::ChoiceOptionsHint ).toStringList();
      if( ed->choices() != choices ) ed->setChoices(choices);
      QList<QVariant> list = qVariantValue<QList<QVariant> >(index.model()->data( index, Qt::EditRole ));
      if( list != ed->values())
          ed->setValues(list);
    }
  }

  virtual void setModelData(QWidget *editor, QAbstractItemModel* model, const QModelIndex& index )
  {
    if(KDMultipleChoicePropertyWidget* ed = qobject_cast<KDMultipleChoicePropertyWidget*>(editor)) {
      QList<QVariant> newc = ed->values();
      QList<QVariant> oldc = qVariantValue<QList<QVariant> >(model->data( index, Qt::EditRole ));
      if (newc != oldc ) {
          model->setData( index, qVariantFromValue(newc), Qt::EditRole );
      }
    }
  }
};

REGISTER_KDPROPERTYEDITOR( KDStringPropertyEditor )
REGISTER_KDPROPERTYEDITOR( KDComboPropertyEditor )
REGISTER_KDPROPERTYEDITOR( KDBooleanPropertyEditor )
REGISTER_KDPROPERTYEDITOR( KDChoicePropertyEditor )
REGISTER_KDPROPERTYEDITOR( KDMultipleChoicePropertyEditor )
#ifndef QT_NO_FILEDIALOG
REGISTER_KDPROPERTYEDITOR( KDFilenamePropertyEditor )
#endif // QT_NO_FILEDIALOG
REGISTER_KDPROPERTYEDITOR( KDDoublePropertyEditor )
REGISTER_KDPROPERTYEDITOR( KDIntPropertyEditor )
REGISTER_KDPROPERTYEDITOR( KDIntSliderPropertyEditor )
#ifndef QT_NO_COLORDIALOG
REGISTER_KDPROPERTYEDITOR( KDColorPropertyEditor )
#endif
#ifndef QT_NO_FONTDIALOG
REGISTER_KDPROPERTYEDITOR( KDFontPropertyEditor )
#endif

#include "kdpropertyeditor.moc"
