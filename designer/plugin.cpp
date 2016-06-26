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

#include <QDesignerCustomWidgetCollectionInterface>
#include <QDesignerCustomWidgetInterface>
#include <QDesignerFormEditorInterface>
#include <QExtensionManager>
#include <QMetaClassInfo>

#include <KDToolsGui/kdcircularprogressindicator.h>
#include <KDToolsGui/kdhelpbutton.h>
#include <KDToolsGui/kddoublespinbox.h>
#include <KDToolsGui/kdlistviewtoolbar.h>
#include <KDToolsGui/kdlockbutton.h>
#include <KDToolsGui/kdsearchlineedit.h>
#include <KDToolsGui/kdlogtextwidget.h>
#include <KDToolsGui/kdtimescrollbar.h>
#include <KDToolsGui/kdtimelinewidget.h>
#include <KDToolsGui/kdcolorbutton.h>

namespace {

template <typename T_Widget>
QWidget * create( QWidget * parent ) {
  return new T_Widget( parent );
}

QString classInfo( const QMetaObject & mo, const char * what ) {
    const int idx = mo.indexOfClassInfo( what );
    if ( idx < 0 )
        return QString();
    return QString::fromLatin1( mo.classInfo( idx ).value() );
}

template <typename T_Widget>
QString classInfo( const char * what ) {
    return classInfo( T_Widget::staticMetaObject, what );
}

struct yes { static const bool value = true;  };
struct no  { static const bool value = false; };

template <typename T_Widget>
struct is_container : no {};

#define SET_IS_CONTAINER( x ) template <> struct is_container<x> : yes {}

template <typename T>
void init( QDesignerCustomWidgetInterface *, QDesignerFormEditorInterface * ) {}

typedef QWidget * (*widget_create_func)( QWidget * );
typedef QString ( *widget_classinfo_func )( const char * what );
typedef void ( *plugin_init_func )( QDesignerCustomWidgetInterface *, QDesignerFormEditorInterface * );

static const struct widget_info {
    const char * name;
    const char * group;
    const char * icon;
    bool isContainer;
    widget_create_func create;
    widget_classinfo_func classInfo;
    plugin_init_func init;
} widgetInfos[] = {
#define WIDGET( x )                                             \
    {                                                           \
        #x,                                                     \
        "KDTools",                                              \
        ":/.__kdab__/kdtools/icons/designer/kdab-small.png",    \
        is_container<x>::value,                                 \
        &create<x>, &classInfo<x>, &init<x>,                    \
    }
    WIDGET( KDLogTextWidget ),
    WIDGET( KDTimeScrollBar ),
    WIDGET( KDTimeLineWidget ),
    WIDGET( KDTimeLineArea ),
    WIDGET( KDColorButton ),
    WIDGET( KDCircularProgressIndicator ),
    WIDGET( KDHelpButton ),
    WIDGET( KDListViewToolBar ),
    WIDGET( KDSpinBox ),
    WIDGET( KDDoubleSpinBox ),
    WIDGET( KDLockButton ),
    WIDGET( KDSearchLineEdit )
#undef WIDGET
};

static const unsigned int numWidgetInfos = sizeof widgetInfos / sizeof *widgetInfos ;

class KDSingleWidgetPlugin : public QObject, public QDesignerCustomWidgetInterface {
    Q_OBJECT
    Q_INTERFACES( QDesignerCustomWidgetInterface )
public:
    explicit KDSingleWidgetPlugin( unsigned int idx_ ) : idx( idx_ ), inited( false ) {}

    QString name() const
        { return QString::fromLatin1( widgetInfos[idx].name ); }
    QString group() const
        { return QString::fromLatin1( widgetInfos[idx].group ); }
    QString toolTip() const
        { return whatsThis(); }
    QString whatsThis() const
        { return widgetInfos[idx].classInfo( "description" ); }
    QString includeFile() const
        { return QString::fromLatin1( widgetInfos[idx].name ); }
    QIcon icon() const
        { return QIcon( QString::fromLatin1( widgetInfos[idx].icon ) ); }
    bool isContainer() const
        { return widgetInfos[idx].isContainer; }
    QWidget * createWidget( QWidget * p )
        { return widgetInfos[idx].create( p ); }
    bool isInitialized() const
        { return inited; }
    void initialize( QDesignerFormEditorInterface * core ) {
        if ( inited )
            return;
        if ( const plugin_init_func f = widgetInfos[idx].init )
            f( this, core );
        inited = true;
    }
    QString codeTemplate() const
        { return QString(); }
    QString domXml() const
        { return QString::fromLatin1( "<widget class=\"%1\" name=\"%2\">\n"
                                      "</widget>\n" ).arg( name(), name().toLower() ); }
protected:
    const unsigned int idx;
    bool inited;
};


class KDWidgetPlugin : public QObject, public QDesignerCustomWidgetCollectionInterface {
    Q_OBJECT
    Q_INTERFACES( QDesignerCustomWidgetCollectionInterface )
public:
    explicit KDWidgetPlugin( QObject * parent=0 );
    QList<QDesignerCustomWidgetInterface*> customWidgets() const { return mCustomWidgets; }
private:
    QList<QDesignerCustomWidgetInterface*> mCustomWidgets;
};


KDWidgetPlugin::KDWidgetPlugin( QObject * p )
  : QObject( p ),
    QDesignerCustomWidgetCollectionInterface(),
    mCustomWidgets()
{
    for ( unsigned int i = 0 ; i < numWidgetInfos ; ++i )
        mCustomWidgets.push_back( new KDSingleWidgetPlugin( i ) );
}

} // anon namespace

#include "plugin.moc"

#include <qplugin.h>
#if QT_VERSION >= 0x040100
Q_EXPORT_PLUGIN2( kdtools_designer_plugin, KDWidgetPlugin )
#else
Q_EXPORT_PLUGIN( KDWidgetPlugin )
#endif
