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

#include <KDToolsGui/KDPropertyView>
#include <KDToolsGui/KDPropertyModel>
#include <KDToolsGui/kdproperty.h>

#include <QtGui>

class Preview : public QWidget {
public:
  explicit Preview( QWidget* parent=0 )
    : QWidget( parent )
  {
    setMinimumSize( 300,300 );
  }

  void setText( const QString& text )
  {
    m_text = text; update();
  }
  void setTextColor( const QColor& color )
  {
    m_textcolor = color; update();
  }
  void setFramePen( const QPen& pen )
  {
    m_framepen = pen;
    update();
  }
  void setFrameBrush( const QBrush& brush )
  {
    m_framebrush = brush;
    update();
  }

protected:
  virtual void paintEvent( QPaintEvent* )
  {
    QPainter p(this);
    QRect framerect = rect().adjusted( 10, 10, -10, -10 );
    p.setPen(m_framepen);
    p.setBrush(m_framebrush);
    p.drawRect( framerect );
    p.setPen( m_textcolor );
    p.drawText( framerect, Qt::AlignCenter|Qt::TextWordWrap, m_text );
  }

private:
  QString m_text;
  QColor m_textcolor;
  QPen m_framepen;
  QBrush m_framebrush;
};

class PropertyView : public QWidget {
  Q_OBJECT
public:
  explicit PropertyView( QWidget* parent = 0 )
    : QWidget(parent)
  {
    /* Children */
    QToolBar* toolbar = new QToolBar;
    m_propertyview = new KDPropertyView;
    m_propertymodel = new KDPropertyModel(this);
    QLabel* helplabel = new QLabel;
    m_helpviewer = new QTextEdit;
    m_preview = new Preview;
    QAction* flataction = toolbar->addAction( style()->standardIcon( QStyle::SP_FileDialogDetailedView ), tr("Toggle Flat Mode") );

    /* Layouts */
    QHBoxLayout* toplayout = new QHBoxLayout(this);
    QVBoxLayout* leftlayout = new QVBoxLayout;
    toplayout->addLayout(leftlayout);
    leftlayout->addWidget(toolbar);
    leftlayout->addWidget(m_propertyview,1);
    leftlayout->addWidget(helplabel);
    leftlayout->addWidget(m_helpviewer);
    toplayout->addWidget(m_preview);
    setLayout(toplayout);

    /* Setup widgets */
    helplabel->setText( tr("&Description:") );
    helplabel->setBuddy(m_helpviewer);
    m_helpviewer->setReadOnly(true);
    m_propertyview->setModel(m_propertymodel);
    flataction->setCheckable(true);

    /* Setup properties */
    KDPropertyCategory* contents = new KDPropertyCategory( tr("Contents") );

    m_textprop = new KDStringProperty( tr("Use the properties on the left hand side to change this label."), tr("Text") );
    m_textprop->setHelpText( tr("This is the string displayed in the frame.") );
    contents->addProperty(m_textprop);

    m_textfont = new KDFontProperty( font(), tr("Font") );
    m_textfont->setHelpText( tr("This is the font for the text displayed in the frame.") );
    contents->addProperty(m_textfont);

    m_textcolor = new KDColorProperty( Qt::black, tr("Color") );
    m_textcolor->setHelpText( tr("This is the color for the text displayed in the frame,") );
    contents->addProperty(m_textcolor);

    KDPropertyCategory* frame = new KDPropertyCategory( tr("Frame") );
    frame->setIcon( style()->standardPixmap(QStyle::SP_MessageBoxInformation) );
    frame->setCheckable(false);

    m_framewidth = new KDIntProperty( 1, tr("Frame Width") );
    m_framewidth->setHelpText( tr("The width of the frame in pixels.") );
    frame->addProperty( m_framewidth );

    m_framecolor = new KDColorProperty( Qt::black, tr("Color") );
    m_framecolor->setHelpText( tr("This is the color for the frame,") );
    frame->addProperty(m_framecolor);

    m_backgroundcolor = new KDColorProperty( palette().color( QPalette::Background ), tr("Background Color") );
    m_backgroundcolor->setHelpText( tr("This is the background color for the frame."));
    frame->addProperty(m_backgroundcolor);

    m_propertymodel->addProperty( contents );
    m_propertymodel->addProperty( frame );

    // initially sync the label and the properties
    m_preview->setText(m_textprop->data() );
    m_preview->setFont(m_textfont->data() );
    m_preview->setTextColor(m_textcolor->data());
    m_preview->setFramePen( QPen( m_framecolor->data(), m_framewidth->data()) );
    m_preview->setFrameBrush(m_backgroundcolor->data() );

    /* Connections */
    connect( flataction, SIGNAL(toggled(bool)),
             m_propertymodel, SLOT(setFlat(bool)));
    connect( m_propertyview->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
             this, SLOT(slotCurrentChanged(QModelIndex,QModelIndex)));
    connect( m_propertymodel, SIGNAL(propertyChanged(KDPropertyInterface*)),
             this, SLOT(slotPropertyChanged(KDPropertyInterface*)));
  }
private Q_SLOTS:
  void slotCurrentChanged(const QModelIndex& idx,const QModelIndex&)
  {
    KDPropertyInterface* p = static_cast<KDPropertyInterface*>(idx.internalPointer());
    if( p ) m_helpviewer->setPlainText( p->helpText() );
  }
  void slotPropertyChanged( KDPropertyInterface* prop = 0 )
  {
    if( prop == m_textprop )
        m_preview->setText(m_textprop->data());
    else if( prop == m_textfont )
        m_preview->setFont(m_textfont->data() );
    else if( prop == m_textcolor )
        m_preview->setTextColor( m_textcolor->data() );
    else if( prop == m_framecolor || prop == m_framewidth )
        m_preview->setFramePen( QPen( m_framecolor->data(), m_framewidth->data()) );
    else if( prop == m_backgroundcolor )
        m_preview->setFrameBrush(m_backgroundcolor->data() );
  }
private:
  KDPropertyView* m_propertyview;
  KDPropertyModel* m_propertymodel;
  QTextEdit* m_helpviewer;
  Preview* m_preview;

  KDStringProperty* m_textprop;
  KDFontProperty* m_textfont;
  KDColorProperty* m_textcolor;

  KDIntProperty* m_framewidth;
  KDColorProperty* m_framecolor;
  KDColorProperty* m_backgroundcolor;
};

int main( int argc, char** argv ) {
    QApplication app( argc, argv );

    PropertyView v;
    v.show();

    return app.exec();
}

#include "main.moc"
