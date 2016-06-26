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

#ifndef __KDTOOLS__GUI__KDTIMELINEWIDGET_H__
#define __KDTOOLS__GUI__KDTIMELINEWIDGET_H__

#include <KDToolsCore/kdtoolsglobal.h>
#include <KDToolsCore/pimpl_ptr.h>

#if QT_VERSION < 0x050000
#include <QtGui/QFrame>
#else
#include <QtWidgets/QFrame>
#endif

QT_BEGIN_NAMESPACE
class QDateTimeEdit;
class QLabel;
class QPainter;
class QScrollBar;
class QStackedWidget;
class QToolButton;
class QDateTime;
class QPalette;
QT_END_NAMESPACE

class KDTimeScrollBar;
class KDTimeLineWidget;

class KDTOOLSGUI_EXPORT KDTimeLineWidgetItem {
    friend class ::KDTimeLineWidget; /* TODO */
    Q_DISABLE_COPY( KDTimeLineWidgetItem  )
public:
    KDTimeLineWidgetItem();
    explicit KDTimeLineWidgetItem( const QDateTime & dt );
    virtual ~KDTimeLineWidgetItem();

    void setDateTime( const QDateTime& dt );
    QDateTime dateTime() const;

    void setPalette( const QPalette& pal );
    const QPalette& palette() const;

    void setMovable( bool m );
    bool isMovable() const;
    bool isCurrent() const;

    KDTimeLineWidget* timeLineWidget() const;

    virtual void paintItem( QPainter* p, const QPoint& pos );

private:
    class Private;
    kdtools::pimpl_ptr<Private> d;
};

class KDTimeLineWidgetPrivate;
class KDTOOLSGUI_EXPORT KDTimeLineWidget : public QWidget {
    Q_OBJECT
    Q_PROPERTY( QDateTime startDateTime READ startDateTime WRITE setStartDateTime )
    Q_PROPERTY( QDateTime endDateTime READ endDateTime WRITE setEndDateTime )
    Q_PROPERTY( QDateTime currentDateTime READ currentDateTime WRITE setCurrentDateTime )
    Q_PROPERTY( QDateTime elapsedDateTime READ elapsedDateTime WRITE setElapsedDateTime )
    Q_PROPERTY( bool snapToTickmarks READ snapToTickmarks WRITE setSnapToTickmarks )
public:
    explicit KDTimeLineWidget( QWidget* parent = 0 );
    virtual ~KDTimeLineWidget();

    QDateTime startDateTime() const;
    QDateTime endDateTime() const;

    QDateTime currentDateTime() const;
    QDateTime elapsedDateTime() const;

    bool snapToTickmarks() const;

    void setDateTimeFormat( const QString& format );
    void setDateTimeFormat( Qt::DateFormat format );

    //void setMinorTickCount( int num );
    //int minorTickCount() const { return m_minorticks; }

    void addItem( KDTimeLineWidgetItem* item );
    void removeItem( KDTimeLineWidgetItem* item );
    int itemCount() const;
    KDTimeLineWidgetItem* item(int idx) const;
    KDTimeLineWidgetItem* currentItem() const;

    QSize sizeHint() const KDAB_OVERRIDE;
    QSize minimumSizeHint() const KDAB_OVERRIDE;

    QDateTime dateTimeAt( int x ) const;
    int position( const QDateTime& dt ) const;

Q_SIGNALS:
    void itemSelected( KDTimeLineWidgetItem* );
    void itemMoved( KDTimeLineWidgetItem* );
    void currentDateTimeChanged(const QDateTime&);
    void elapsedDateTimeChanged(const QDateTime&);

public Q_SLOTS:
    void setStartDateTime( const QDateTime& );
    void setEndDateTime( const QDateTime& );

    void setSnapToTickmarks(bool);

    void setCurrentDateTime( const QDateTime& );

    void nextCurrentDateTime();
    void previousCurrentDateTime();

    void setElapsedDateTime( const QDateTime& );

    void setCurrentItem( KDTimeLineWidgetItem* );
    void nextCurrentItem();
    void previousCurrentItem();

protected:
    void paintEvent( QPaintEvent * ) KDAB_OVERRIDE;
    void mousePressEvent( QMouseEvent * ) KDAB_OVERRIDE;
    void mouseMoveEvent( QMouseEvent * ) KDAB_OVERRIDE;
private:
    class Private;
    kdtools::pimpl_ptr<Private> d;
};


class KDTOOLSGUI_EXPORT KDTimeLineArea : public QFrame {
    Q_OBJECT
    Q_PROPERTY( int zoomValue READ zoomValue WRITE setZoomValue )
    Q_PROPERTY( QDateTime currentDateTime READ currentDateTime WRITE setCurrentDateTime )
    Q_PRIVATE_SLOT( d, void slotScrollValueChanged( int ) )
    Q_PRIVATE_SLOT( d, void slotZoomValueChanged( int ) )
public:
    explicit KDTimeLineArea( QWidget* parent = 0 );
    virtual ~KDTimeLineArea();

    void setTimeLineWidget( KDTimeLineWidget* tl );
    KDTimeLineWidget* timeLineWidget() const;
    KDTimeScrollBar* scrollBar() const;

    QDateTime currentDateTime();

    int zoomValue() const;

Q_SIGNALS:
    void zoomRangeChanged( int min, int max );

public Q_SLOTS:
    void setStartDateTime( const QDateTime& dt );
    void setEndDateTime( const QDateTime& dt );
    void setScrollValue(int);
    void setZoomValue(int);

    void displayDateTime( const QDateTime& start, const QDateTime& end );
    void ensureVisible(const QDateTime& dt);
    void setCurrentDateTime( const QDateTime& );
    void setElapsedDateTime( const QDateTime& );

    void previousItem();
    void nextItem();

protected:
    bool eventFilter( QObject * , QEvent * ) KDAB_OVERRIDE;
    void keyPressEvent( QKeyEvent * ) KDAB_OVERRIDE;
private:
    class Private;
    kdtools::pimpl_ptr<Private> d;
};

#endif /* __KDTOOLS__GUI__KDTIMELINEWIDGET_H__ */

