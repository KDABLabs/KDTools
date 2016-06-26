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

#include "kdshortcutbar.h"
#include "kdshortcutsplitterhandle_p.h"

#include <QApplication>
#include <QBasicTimer>
#include <QButtonGroup>
#include <QDesktopWidget>
#include <QLayout>
#include <QMenu>
#include <QPointer>
#include <QStackedWidget>
#include <QStyleOptionButton>
#include <QStylePainter>
#include <QTimerEvent>
#include <QToolButton>

/*!
  \defgroup KDShortcutBar KDShortcutBar
  \ingroup gui
*/

class KDShortcut::Private {
    friend class ::KDShortcutBar;
    friend class ::KDShortcut;
    //friend class ::KDShortcutBar::Private;
    KDShortcut * const q;
    Q_DISABLE_COPY(Private)
public:
    explicit Private( KDShortcut * qq )
        : q( qq ),
          shortcutbar( 0 ),
          widget( 0 ),
          visible( true ),
          tbutton( 0 ),
          menuaction( 0 )
    {

    }
    ~Private() {}

private:
    void slotPressed();
    void slotReleased();

private:
    void snapMoveSplitter( int pos );

//private:
public:
    KDShortcutBar* shortcutbar;
    QWidget* widget;

    QIcon statusicon;

    QPointer<QMenu> menu;
    QBasicTimer timer;
    bool visible;

    QToolButton* tbutton;
    QAction* menuaction;

    void setShortcutBar( KDShortcutBar* w ) { shortcutbar = w; }
    void updateToolButton() {
        if( !tbutton ) return;
        if( tbutton->icon().serialNumber() != q->icon().serialNumber() ) tbutton->setIcon(q->icon());
        if( tbutton->toolTip() != q->text() ) tbutton->setToolTip(q->text());
        if( tbutton->isEnabled() != q->isEnabled() ) tbutton->setEnabled(q->isEnabled());
        if( tbutton->menu() != q->menu() ) tbutton->setMenu(q->menu());
    }

    QSize iconSize() const {
        if( shortcutbar ) return shortcutbar->iconSize();
        else {
            int extend = q->style()->pixelMetric(QStyle::PM_SmallIconSize);
            return QSize(extend,extend);
        }
    }

    QStyleOptionButton getStyleOption() const {
        QStyleOptionButton opt;
        opt.init(q);
        opt.text = q->text();
        opt.icon = q->icon();
        opt.iconSize = iconSize();
        QPalette pal = q->palette();
        pal.setColor( QPalette::Button, pal.color(QPalette::Window));
        opt.palette = pal;
        if( q->hasFocus() ) opt.state |= QStyle::State_HasFocus;
        return opt;
    }
};

/***********************************************
 *          KDShortcut
 **********************************************/
/*! \class KDShortcut KDShortcut
  \ingroup KDShortcutBar
  \brief An instance of this class represents a single
  category in a KDShortcutBar.
*/

/*! Contructor. Creates a KDShortcut
  with default values for all attributes and associated
  with widget \a widget. The class can be used as is:

  catwidget->addCategory( new KDShortcut( new QTreeList ) );

  or it can be subclassed for greater flexibility.
*/
KDShortcut::KDShortcut( QWidget* widget, QWidget* parent )
  : QAbstractButton(parent),
    d( new Private( this ) )
{
  Q_ASSERT( widget );
  d->widget = widget;
  setCheckable(true);
  setAutoExclusive(true);
  setFocusPolicy(Qt::StrongFocus);
  setText( d->widget->windowTitle() );
  setIcon( d->widget->windowIcon() );
  connect( this, SIGNAL(pressed()),
	   this, SLOT(slotPressed()));
  connect( this, SIGNAL(released()),
	   this, SLOT(slotReleased()));
}

/*! \fn KDShortcut::widget() const
  Returns a pointer to the widget associated with
  this category.
*/
QWidget* KDShortcut::widget() const { return d->widget; }


/*! Destructor. If this shortcut belongs to a KDShortcutBar,
  it is removed from the widget before it is destroyed.
*/
KDShortcut::~KDShortcut()
{
  if( d->shortcutbar ) d->shortcutbar->removeShortcut(this);
  delete d->widget;
}
/*! \fn KDShortcut::text() const
  Returns the text for this category.
*/

/*! \returns a pointer to the KDShortcutBar that
  contains this KDShortcut or 0 if the shortcut is not
  on any bar.
*/
KDShortcutBar* KDShortcut::shortcutBar() const { return d->shortcutbar; }

/*! Set the status icon for this shortcut.
 This is the optional icon that is is displayed on the
 left side of the button. The default value is no icon. */
void KDShortcut::setStatusIcon( const QIcon& icon )
{
  d->statusicon = icon;
}

/*! \fn KDShortcut::statusIcon() const
  Returns the status icon of this shortcut. This is the optional
  icon that is is displayed on the left side of the button.
*/
QIcon KDShortcut::statusIcon() const { return d->statusicon; }

/*! Set the visibility of this shortcut to \a v. Visible
  shortcuts are displayed as buttons on a KDShortcutBar,
  invisible shortcuts are hidden from the normal view and only
  shown in a menu that allows the user to set visibility for each
  shortcut. This way the user can customize the view to contain
  only a selection of a large number of shortcuts. Default is
  true. */
void KDShortcut::setShortcutVisible( bool v )
{
  d->visible = v;
  /* TODO: update container */
}
/*! \fn bool KDShortcut::isShortcutVisible() const
  Returns true is the shortcut is currently visible.
  \see KDShortcut::setShortcutVisible( bool )
*/
bool KDShortcut::isShortcutVisible() const { return d->visible; }

/*! Returns true is this shortcut is currently selected.
 */
bool KDShortcut::isCurrent() const
{
  if( d->shortcutbar ) return d->shortcutbar->currentShortcut() == this;
  else return false;
}

void KDShortcut::Private::slotReleased()
{
  timer.stop();
}

void KDShortcut::Private::slotPressed()
{
  emit q->pressed(q);
  if( q->menu() ) timer.start( q->style()->styleHint(QStyle::SH_ToolButton_PopupDelay, 0, q), q );
}

void KDShortcut::timerEvent(QTimerEvent* ev)
{
  if( ev->timerId() == d->timer.timerId() ) {
    showMenu();
  }
}

/*! Show the popup menu set on this KDShortcut */
void KDShortcut::showMenu()
{
  d->timer.stop();
  QPointer<QMenu> m = menu();
  if( m.isNull() ) return;
  QPoint p;
  QRect screen = qApp->desktop()->availableGeometry(this);
  QSize sh = m->sizeHint();
  QRect r = rect();
  /* Mimic behaviour of QToolButton, but with
   menu flush to the right side if screen
   space allows it */
  if(mapToGlobal(QPoint(r.right(), 0)).x() + sh.width() <= screen.width()) {
    p = mapToGlobal(r.topRight());
  } else {
    p = mapToGlobal(QPoint(-sh.width(),0));
  }
  //p.rx() -= sh.width();
  m->exec(p);
}

/*! Sets a popup menu on this shortcut. The shortcut object takes
 ownership of the menu. */
void KDShortcut::setMenu( QMenu* menu )
{
  if( d->menu != menu ) delete d->menu;
  d->menu = menu;
}

/*! Subclass and override this method to provide a custom popup menu
  for this shortcut. The default implementation returns the value
  set with setMenu(QMenu*), which is NULL by default.
*/
QMenu* KDShortcut::menu() const
{
  return d->menu;
}

QSize KDShortcut::sizeHint() const
{
  ensurePolished();
  QFontMetrics fm = fontMetrics();
  QStyleOptionButton opt = d->getStyleOption();

  int w = 0;
  int h = 0;
  if( !icon().isNull() ) {
    int ih = opt.iconSize.height();
    int iw = opt.iconSize.width() + 4;
    w += iw;
    h = qMax(h, ih);
  }
  // Always make space for the menu indicator
  w += style()->pixelMetric(QStyle::PM_MenuButtonIndicator, &opt, this);
  QSize sz = fm.size(Qt::TextShowMnemonic, text());
  if( w == 0 ) w=sz.width();
  h = qMax(h, sz.height());
  return (style()->sizeFromContents(QStyle::CT_PushButton, &opt, QSize(w, h), this).
	  expandedTo(QApplication::globalStrut()));
}

/*! Paints the button associated with this shortcut.
  Subclass and override to create shortcuts with custom look. */
void KDShortcut::paintEvent(QPaintEvent*)
{
  /* There are no good places to hook into setText() etc.
     so we have to update the toolbutton from here */
  d->updateToolButton();


  QStylePainter p(this);
  QStyleOptionButton option = d->getStyleOption();

  Q_ASSERT(d->shortcutbar);
  if( isCurrent() && style()->styleHint(QStyle::SH_ToolBox_SelectedPageTitleBold, 0, this)) {
    QFont f(font());
    f.setBold(true);
    p.setFont(f);
  }
  QRect tr = option.rect;
  QRect pmr;
  QRect spmr;
  int mbi = style()->pixelMetric(QStyle::PM_MenuButtonIndicator, &option, this);
  QPixmap pm = icon().pixmap(iconSize(),
			     d->shortcutbar->isEnabled() ? QIcon::Normal : QIcon::Disabled);
  if( !pm.isNull() ) {
    pmr = pm.rect().translated( 4, (option.rect.height()-pm.height())/2 );
    tr.adjust( pmr.width()+8, 0,0,0 );
  } else {
    tr.adjust( 8, 0,0,0 );
  }
  QPixmap spm = statusIcon().pixmap(iconSize(),
                                    d->shortcutbar->isEnabled() ? QIcon::Normal : QIcon::Disabled);
  if( !spm.isNull() ) {
    spmr = spm.rect().translated( option.rect.width()-spm.width()-8-mbi,
				  (option.rect.height()-pm.height())/2 );
    tr.adjust( 0, 0,-spmr.width()-8-mbi,0 );
  }
  QString ctext = text();
  QString txt;
  QFontMetrics fm = fontMetrics();
  if (fm.width(ctext) < tr.width()) {
    txt = ctext;
  } else {
    txt = ctext.left(1);
    int ew = fm.width( QLatin1String( "..." ) );
    int i = 1;
    while(fm.width(txt) + ew +
          fm.width(ctext[i])  < tr.width())
      txt += ctext[i++];
    txt += QLatin1String( "..." );
  }

  QStyleOptionButton opt = option;
  opt.state = QStyle::State_AutoRaise;
  if( isCurrent() ) {
    opt.state |= QStyle::State_Sunken|QStyle::State_Selected|QStyle::State_Enabled;
  } else {
    opt.state |= QStyle::State_Enabled|QStyle::State_Raised;
  }

  //p.drawPrimitive(QStyle::PE_FrameButtonBevel, opt );
  p.drawPrimitive(QStyle::PE_PanelButtonTool, opt );

  if(menu()) {
    QRect ir = opt.rect;
    QStyleOptionButton newOpt = opt;
    newOpt.rect = QRect(ir.right() - mbi, ir.height() - 20, mbi, 20);
    p.drawPrimitive(QStyle::PE_IndicatorArrowRight, newOpt);
  }

  if(!pmr.isNull())
    p.drawPixmap(pmr, pm);
  if(!spmr.isNull())
    p.drawPixmap(spmr, spm);

  if( option.state & QStyle::State_HasFocus ) {
    QStyleOptionFocusRect foption;
    foption.init(this);
    foption.palette = palette();
    foption.backgroundColor = palette().color(QPalette::Window);
    foption.rect = style()->subElementRect( QStyle::SE_PushButtonFocusRect, &option, this );
    p.drawPrimitive(QStyle::PE_FrameFocusRect, foption);
  }

  opt.rect = tr;
  p.drawItemText( tr, Qt::AlignLeft|Qt::AlignVCenter,
		  palette(), isEnabled(),
		  txt );
}

/* internal */
class KDShortcutToolButton : public QToolButton {
  Q_OBJECT
public:
  explicit KDShortcutToolButton( KDShortcut* sc, QWidget* parent = 0 )
    : QToolButton(parent),
      m_shortcut(sc)
  {
    setIcon( sc->icon() );
    setText( sc->text() );
    setToolTip( sc->text() );
    setMenu( sc->menu() );
    setPopupMode( QToolButton::DelayedPopup );
    setCheckable(true);
    setAutoExclusive(true);
    connect( this, SIGNAL(clicked()),
             this, SLOT(slotClicked()));
  }

Q_SIGNALS:
  void clicked( KDShortcut* );
private Q_SLOTS:
  void slotClicked()
  {
    emit clicked(m_shortcut);
  }
private:
  KDShortcut* m_shortcut;
};

/* internal replacement for QToolBar that works better
   for our purposes */
class KDShortcutToolBar : public QFrame {
    Q_OBJECT
public:
  explicit KDShortcutToolBar(QWidget* parent = 0)
      : QFrame(parent),
        m_islayedout(false)
  {
    setFrameStyle( Raised|Panel );
    m_menubutton = new QToolButton(this);
    m_menubutton->setIcon( style()->standardPixmap(QStyle::SP_ToolBarHorizontalExtensionButton) );
    m_menubutton->setPopupMode( QToolButton::InstantPopup );
    m_menubutton->setAutoRaise(true);
  }

  void setMenu( QMenu* menu ) {
    m_menubutton->setMenu(menu);
  }
  QMenu* menu() const { return m_menubutton->menu(); }

  void addButton( QToolButton* button ) {
    insertButton(m_buttons.size(),button);
  }
#if 0
  void insertButton( QToolButton* before, QToolButton* button ) {
    if( before ) {
      int idx = m_buttons.indexOf(before);
      insertButton(idx,button);
   }
  }
#endif
  void insertButton( int idx, QToolButton* button ) {
    button->setParent(this);
    button->setAutoRaise(true);
    m_buttons.insert(idx,button);
    doDelayedLayout();
    update();
  }
  void removeButton( QToolButton* button )
  {
    m_buttons.removeAll(button);
    doDelayedLayout();
    update();
  }

  virtual QSize sizeHint() const
  {
    ensurePolished();
    int h = 0;
    int w = 0;
    for( QList<QToolButton*>::const_iterator it = m_buttons.begin();
	 it != m_buttons.end(); ++it ) {
      QSize s = (*it)->sizeHint();
      w += s.width();
      h = qMax(h,s.height());
    }
    h = qMax(h,m_menubutton->sizeHint().height());
    return QSize(w+2*frameWidth(),h+2*frameWidth()).expandedTo( QApplication::globalStrut() );
  }
  virtual QSize minimumSizeHint() const {
    QSize s = sizeHint();
    return QSize( m_menubutton->sizeHint().width()+2*frameWidth(), s.height() );
  }

protected:
  virtual bool event(QEvent* ev)
  {
    if( ev->type() == QEvent::LayoutRequest ) {
      doLayout();
      return true;
    } else if( ev->type() == QEvent::ChildRemoved ) {
      removeButton(static_cast<QToolButton*>(static_cast<QChildEvent*>(ev)->child()));
      return true;
    } else return QFrame::event(ev);
  }
  virtual void resizeEvent(QResizeEvent*)
  {
    doLayout();
  }
private:
  void doDelayedLayout()
  {
    if( m_islayedout ) {
      m_islayedout = false;
      QApplication::postEvent( this, new QEvent( QEvent::LayoutRequest ) );
    }
  }
  void doLayout()
  {
    int w = frameWidth();
    for( QList<QToolButton*>::iterator it = m_buttons.begin();
	 it != m_buttons.end(); ++it ) {
      QToolButton* widget = *it;
      Q_ASSERT(widget);
      if( !widget->isVisibleTo(this) ) continue;
      if( w+widget->sizeHint().width() < width()-2*frameWidth()-m_menubutton->width() ) {
	widget->resize( widget->sizeHint().width(), height()-2*frameWidth() );
	widget->move( w, frameWidth() );
      } else {
	widget->move(width(),height()); // Move widgets away
      }
      w += widget->width();
    }
    m_menubutton->resize( m_menubutton->sizeHint().width(), height()-2*frameWidth() );
    m_menubutton->move( width()-m_menubutton->width()-frameWidth(), frameWidth() );
    m_islayedout = true;
  }

  QList<QToolButton*> m_buttons;
  QToolButton* m_menubutton;
  bool m_islayedout;
};

/* internal */
class KDShortcutBar::Private : public QWidget {
  Q_OBJECT
  friend class ::KDShortcutBar;
  friend class ::KDShortcut;
  friend class ::KDShortcut::Private;
public:
  explicit Private( QWidget* parent = 0 )
    : QWidget(parent),
      placement(KDShortcutBar::WidgetAbove),
      stack(0),
      handle(0),
      iconsize(style()->pixelMetric(QStyle::PM_SmallIconSize),
	       style()->pixelMetric(QStyle::PM_SmallIconSize)),
      toolbar(0),
      separator(0),
      menu(0),
      catmenu(0),
      buttongroup(0),
      current(0),
      numvisible(1)
  {
  }

  bool event(QEvent* ev)
  {
    if( ev->type() == QEvent::LayoutRequest ) {
      doLayout();
      return false;
    } else return QWidget::event(ev);
  }

  void resizeEvent( QResizeEvent* )
  {
    doLayout();
  }

  void doTopLayout(QWidget* w) {
    delete w->layout();
    QVBoxLayout* l = new QVBoxLayout(w);
    l->setSpacing(0);
    l->setMargin(0);
    if( placement == KDShortcutBar::WidgetAbove ) {
      l->addWidget(stack);
      l->addWidget(handle);
      l->addWidget(this);
      l->addWidget(toolbar);
    } else {
      l->addWidget(this);
      l->addWidget(toolbar);
      l->addWidget(handle);
      l->addWidget(stack);
    }
    w->setLayout(l);
  }

  void doLayout()
  {
    int c = 0;
    int h = 0;
    delete buttongroup;
    buttongroup = new QButtonGroup(this);
    QList<KDShortcut*>::iterator it;
    for( it = shortcuts.begin();
	 it != shortcuts.end(); ++it ) {
      KDShortcut* i = *it;
      if( c >= numvisible ) break;
      if( i->isShortcutVisible() ) {
	i->move(0,h);
	i->resize( width(), i->sizeHint().height() );
	h+=i->sizeHint().height();
	i->show();
	buttongroup->addButton(i);
	//delete i->d->tbutton; i->d->tbutton = 0;
	if( i->d->tbutton ) i->d->tbutton->hide();
	++c;
      } else {
	i->hide();
      }
    }
    for( ;it != shortcuts.end();++it ) {
      KDShortcut* i = *it;
      if( i->isShortcutVisible() ) {
	if( !i->d->tbutton ) {
	  i->d->tbutton = new KDShortcutToolButton(i);
	  toolbar->addButton(i->d->tbutton);
	  connect( i->d->tbutton, SIGNAL(clicked(KDShortcut*)),
		   parentWidget(), SLOT(setCurrentShortcut(KDShortcut*)));
	}
	i->d->tbutton->setIcon( i->icon() );
	i->d->tbutton->show();
	buttongroup->addButton(i->d->tbutton);
      } else {
	//delete i->d->tbutton; i->d->tbutton = 0;
	if( i->d->tbutton ) i->d->tbutton->hide();
      }
      i->hide();
    }

    setFixedHeight( h );
    if( shortcuts.isEmpty() ) handle->setStepSize( toolbar->sizeHint().height() );
    else handle->setStepSize( shortcuts.front()->sizeHint().height() );
  }

public Q_SLOTS:
  void update(KDShortcut* sc)
  {
    for( QList<KDShortcut*>::iterator it = shortcuts.begin();
	 it != shortcuts.end(); ++it ) {
      if( (*it) == sc ) {
	(*it)->update();
	if( (*it)->d->tbutton ) (*it)->d->tbutton->update();
      }
    }
  }
  void slotMenuActionTriggered(QAction* action) {
    for( QList<KDShortcut*>::iterator it = shortcuts.begin();
	 it != shortcuts.end(); ++it ) {
      KDShortcut* i = *it;
      if( i->d->menuaction == action ) {
	i->setShortcutVisible(action->isChecked());
	doLayout();
	break;
      }
    }
  }
public: // ### PUBLIC????

  KDShortcutBar::WidgetPlacement placement;
  QStackedWidget* stack;
  KDShortcutSplitterHandle* handle;
  QSize iconsize;

  KDShortcutToolBar* toolbar;
  QAction* separator;
  QMenu* menu;
  QMenu* catmenu;

  QList<KDShortcut*> shortcuts;
  QButtonGroup* buttongroup;
  KDShortcut* current;
  int numvisible;
};

/***********************************************
 *          KDShortcutBar
 **********************************************/
/*! \class KDShortcutBar KDShortcutBar
  \ingroup KDShortcutBar
  \brief This widget provides a vertical list of buttons,
  one for each shortcut, and an area that displays
  a widget for each shortcut. The list of buttons
  and the widget area are separated by a splitter.
  The list of buttons can be contracted and expanded
  to display more or less buttons by the user.

  Functionally it is similar to QToolBox but more
  advanced in several ways. This type of widget was
  popularized by Microsoft Outlook, which is also responsible
  for the name "ShortcutBar".

  \image HTML kdshortcutbar.png

*/

/*! Constructor. Creates an empty KDShortcutBar with parent
  \a parent. */
KDShortcutBar::KDShortcutBar(QWidget* parent)
  : QWidget(parent), d(new Private)
{
  d->stack = new QStackedWidget;
  d->handle = new KDShortcutSplitterHandle;
  d->handle->setIncrementFromBottom(d->placement==WidgetAbove);
  d->toolbar = new KDShortcutToolBar;
  //d->toolbar->setMovable(false);
  //d->toolbar->setOrientation( Qt::Horizontal );
  d->toolbar->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
  //d->separator = d->toolbar->addSeparator();
  d->menu = new QMenu;
  d->menu->addAction( tr("Show &More Buttons"), this, SLOT(showMoreShortcuts()) );
  d->menu->addAction( tr("Show &Fewer Buttons"), this, SLOT(showFewerShortcuts()) );
  d->catmenu = d->menu->addMenu( tr("&Add or Remove Buttons") );
  d->toolbar->setMenu(d->menu);
  connect( d->handle, SIGNAL(splitterMovedUp()),
	   this, SLOT(showMoreShortcuts()));
  connect( d->handle, SIGNAL(splitterMovedDown()),
	   this, SLOT(showFewerShortcuts()));
  connect( d->menu, SIGNAL(triggered(QAction*)),
	   d.get(), SLOT(slotMenuActionTriggered(QAction*)));
  d->doTopLayout(this);
}

/*! Destructor. Deletes all contained shortcuts and their
  associated widgets. */
KDShortcutBar::~KDShortcutBar()
{

}

/*! Controls the layout of the widget. If \a p is WidgetAbove,
  the displayed widget will be shown above the list of buttons,
  and if \a p is WidgetBelow, the widget will display below
  the buttons. */
void KDShortcutBar::setWidgetPlacement( WidgetPlacement p)
{
  WidgetPlacement old = d->placement;
  d->placement = p;
  if( p != old ) {
    d->handle->setIncrementFromBottom(p==WidgetAbove);
    d->doTopLayout(this);
    d->doLayout();
  }
}

/*! Returns the current widget placement property
  of this widget. \see setWidgetPlacement
*/
KDShortcutBar::WidgetPlacement KDShortcutBar::widgetPlacement() const
{
  return d->placement;
}

/*! Returns the size currently used for shortcut icons */
QSize KDShortcutBar::iconSize() const
{
  return d->iconsize;
}

/*! Set the size for the icons used on the shortcut
  buttons. The default value is taken from the current
  QStyle's PM_SmallIconSize. Note that this will not
  affect the icons of the shortcuts folded to the bottom
  toolbar -- those will always be small. */
void KDShortcutBar::setIconSize( const QSize& size )
{
  d->iconsize = size;
  update();
}

/*! Adds a shortcut to this widget. The KDShortcutBar
  becomes the owner of \a sc.
*/
void KDShortcutBar::addShortcut( KDShortcut* sc )
{
  insertShortcut(sc,0);
}

/*! Removes a shortcut. The widget releases ownership
  of \a sc and it is the responsibility of the caller to
  delete it.
*/
void KDShortcutBar::removeShortcut( KDShortcut* sc )
{
  Q_ASSERT(sc);
  for( QList<KDShortcut*>::iterator it = d->shortcuts.begin();
       it != d->shortcuts.end(); ) {
    if( (*it) == sc ) {
      delete (*it)->d->tbutton; (*it)->d->tbutton = 0;
      delete (*it)->d->menuaction; (*it)->d->menuaction = 0;
      d->stack->removeWidget((*it)->widget());
      (*it)->d->setShortcutBar(0);
      (*it)->setParent(0);
      it = d->shortcuts.erase(it);
    } else {
      ++it;
    }
  }
}

/*! Inserts a shortcut \a sc into this widget before \a before in the list.
  The KDShortcutBar becomes the owner of \a sc.
*/
void KDShortcutBar::insertShortcut( KDShortcut* sc, KDShortcut* before )
{
  Q_ASSERT(sc);

  QList<KDShortcut*>::iterator before_it = d->shortcuts.end();
  if( before ) for( QList<KDShortcut*>::iterator it = d->shortcuts.begin();
		    it != d->shortcuts.end(); ++it ) {
    if( (*it) == before ) {
      before_it = it;
      break;
    }
  }

  if( before_it == d->shortcuts.end() ) {
    QAction* ma = d->catmenu->addAction( sc->text() );
    ma->setCheckable(true);
    ma->setChecked(sc->isShortcutVisible());
    sc->d->menuaction = ma;
    d->shortcuts.push_back(sc);
  } else {
    QAction* ma = d->catmenu->addAction( sc->text() );
    ma->setCheckable(true);
    ma->setChecked(sc->isShortcutVisible());
    sc->d->menuaction = ma;
    d->shortcuts.insert(before_it,sc);
  }
  sc->d->setShortcutBar(this);
  sc->setParent(d.get());
  d->stack->addWidget(sc->widget());
  connect( sc, SIGNAL(pressed(KDShortcut*)),
	   this, SLOT(setCurrentShortcut(KDShortcut*)));
  d->doLayout();
}

/*! \returns the number of shortcuts in total (including hidden
  and folded ones) in this widget */
int KDShortcutBar::numShortcuts() const
{
  return d->shortcuts.size();
}

/*! \returns the number of visible shortcuts (excluding hidden and folded ones)
  in this widget */
int KDShortcutBar::numShortcutsVisible() const
{
  return d->numvisible;
}

/*! \returns the shortcut at \a idx. Indexes start at zero. */
KDShortcut* KDShortcutBar::shortcutAt(int idx) const
{
  return d->shortcuts.at(idx);
}

/*! Sets the currently selected shortcut to \a sc. */
void KDShortcutBar::setCurrentShortcut(KDShortcut* sc)
{
  KDShortcut* old = d->current;
  d->current = sc;
  if( old != sc ) {
    d->update(old);
  }
  d->stack->setCurrentWidget(sc->widget());
  d->update(sc);
  emit shortcutSelected(sc);
}

/*! \return the currently selected shortcut */
KDShortcut* KDShortcutBar::currentShortcut() const
{
  return d->current;
}

/*! Calling this slot will cause the widget to unfold the
  leftmost folded shortcut and make it visible as a button
  in the list. This will cause the splitter to move and leave
  less space for the client widget. If there are no folded
  shortcuts, it does nothing.
*/
void KDShortcutBar::showMoreShortcuts()
{
  if( d->numvisible < d->shortcuts.size() )  {
    d->numvisible++;
    d->doLayout();
    QApplication::sendPostedEvents(this,0);
  }
}

/*! Calling this slot will cause the widget to fold the
  bottommost displayed shortcut and make it visible as a
  toolbutton in the toolbar for folded shortcuts.
  This will cause the splitter to move and leave
  more space for the client widget. If there are no unfolded
  shortcuts, it does nothing. \see showMoreShortcuts().
*/
void KDShortcutBar::showFewerShortcuts()
{
  if( d->numvisible > 0 )  {
    d->numvisible--;
    d->doLayout();
    QApplication::sendPostedEvents(this,0);
  }
}

/*! \returns a QByteArray containing information
  about which shortcuts are visible/invisible and how
  many shortcuts are unfolded. Use this to make the
  user's configuration of the shortcutbar persistent.
*/
QByteArray KDShortcutBar::saveState() const
{
  QByteArray ba;
  QDataStream ds(&ba, QIODevice::WriteOnly);
  ds << 1; /* version */
  ds << numShortcutsVisible();
  int current = -1;
  QVector<bool> lst;
  const int numSC = numShortcuts();
  lst.reserve( numSC );
  for( int i = 0; i < numSC; ++i ) {
    lst << shortcutAt(i)->isShortcutVisible();
    if( shortcutAt(i)->isCurrent() ) current = i;
  }
  ds << current;
  ds << lst;
  return ba;
}

/*! Restore the state of a shortcutbar. \a ba should be
  a QByteArray obtained from KDShortcutBar::saveState() const.
  \returns true if the state could be restored and false otherwise.
*/
bool KDShortcutBar::restoreState( const QByteArray& ba)
{
  QDataStream ds(ba);
  int version;
  ds >> version;
  if(version!=1) {
    //qWarning() << "KDShortcutBar::restoreState() detected version " << version << ", expected version 1";
    return false;
  }
  int numvisible;
  ds >> numvisible;
  int current;
  ds >> current;
  QVector<bool> lst;
  ds >> lst;
  if( numvisible >= 0 ) d->numvisible = numvisible;
  //qDebug() << "KDShortcutBar::restoreState(), numvisible="<<numvisible<<", current="<<current<<", lst="<<lst;
  for( int i = 0; i < lst.size() && i < numShortcuts(); ++i ) {
    shortcutAt(i)->setVisible(lst[i]);
    if( i == current ) setCurrentShortcut(shortcutAt(i));
  }
  return true;
}

#include "kdshortcutbar.moc"
#include "moc_kdshortcutbar.cpp"
