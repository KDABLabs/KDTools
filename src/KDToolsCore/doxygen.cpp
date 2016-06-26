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

/*!
  \defgroup raii RAII

  Classes that help write exception-safe code.
*/

/*!
  \defgroup  smartptr Smart Pointers

  Smart pointer classes.
*/

/*!
  \defgroup core KDToolsCore

  Tools to match QtCore functionality.
*/

/*!
  \defgroup gui KDToolsGui

  Tools to match QtGui functionality.
*/

/*!
  \defgroup macwidgets Mac Widgets
  \ingroup gui

  Widgets ubiquitous on OS X, but missing in Qt.
*/

/*!
  \defgroup cpp11_features C++11 Feature Test Macros
  \ingroup core

  Macros with which to check for the availability of certain C++11
  features in the compiler.
*/

/*!

  \mainpage KD Tools Reference Documentation

  \image html kdtools.png
  \image html screenshots.png

  KD Tools aims to ease the daily work of Qt programmers by providing
  a number of well-designed, easy-to-use widgets and non-gui classes.

  KD Tools are lifted directly from experience gathered in our daily
  work as consultants and from working on our own products. Components
  range from \link raii small convenience classes\endlink that are
  designed to make recurring tasks easier and/or safer to handle to
  sophisticated view systems, such as KDPropertyView and
  KDTimeLineWidget, that can save you man-weeks of development.

  Most components included were not specifically developed for KD
  Tools, but have repeatedly shown up in our own projects, proving
  their usefulness and undergoing several iterations before being
  included in a release. KD Tools gives you access to the toolbox the
  Qt professionals at KDAB use every day.

  KD Tools is currently packaged into sets of libraries corresponding
  to the Qt libraries QtCore, QtGui, etc. Usage is as simple as
  \code
  CONFIG += kdtools
  KDTOOLS += core gui
  \endcode
  if you use qmake.

  A list of changes and new features can be found on the \link whatsnew Changes\endlink page.
*/

/*!
  \page whatsnew Changes

  \li \link whatsnew23 New features in v2.3\endlink
  \li \link whatsnew22 New features in v2.2\endlink
*/

/*!
  \page whatsnew22 Classes, Functions, Properties, etc. new in KD Tools 2.2

  Links to new classes, functions, properties, and other items introduced in KD Tools 2.2.

  \li \link newclasses22 New Classes\endlink
  \li \link newmethods22 New Member Functions\endlink
  \li \link newproperties22 New Properties\endlink

  \section newclasses22 New Classes

  \li KDEmailValidator
  \li KDGenericFactory
  \li KDThreadRunner
  \li KDVariantConverter

  \section newmethods22 New Member Functions

  \subsection KDLockedSharedMemoryArray

  \li KDLockedSharedMemoryArray::iterator KDLockedSharedMemoryArray::begin()
  \li KDLockedSharedMemoryArray::const_iterator KDLockedSharedMemoryArray::begin() const
  \li KDLockedSharedMemoryArray::iterator KDLockedSharedMemoryArray::end()
  \li KDLockedSharedMemoryArray::const_iterator KDLockedSharedMemoryArray::end() const
  \li KDLockedSharedMemoryArray::reverse_iterator KDLockedSharedMemoryArray::rbegin()
  \li KDLockedSharedMemoryArray::const_reverse_iterator KDLockedSharedMemoryArray::rbegin() const
  \li KDLockedSharedMemoryArray::reverse_iterator KDLockedSharedMemoryArray::rend()
  \li KDLockedSharedMemoryArray::const_reverse_iterator KDLockedSharedMemoryArray::rend() const
  \li KDLockedSharedMemoryArray::size_type KDLockedSharedMemoryArray::size() const
  \li T& KDLockedSharedMemoryArray::back()
  \li const T& KDLockedSharedMemoryArray::back() const

  \section newproperties22 New Properties

  \subsection KDLogTextWidget

  \li KDLogTextWidget::minimumVisibleColumns
*/

/*!
  \page whatsnew23 Classes, Functions, Properties, etc. new in KD Tools 2.3

  Links to new classes, functions, properties, and other items introduced in KD Tools 2.3.

  \li \link newlibraries23 New Libraries\endlink
  \li \link newclasses23 New Classes\endlink
  \li \link newmethods23 New Member Functions\endlink
  \li \link newproperties23 New Properties\endlink
  \li \link newmacros23 New Macros\endlink
  \li \link changes23 Other Changes\endlink

  \section newlibraries23 New Libraries

  \section newclasses23 New Classes

  \li KDSemaphoreReleaser - celebrating our smallest class yet!
  \li KDPoint - Point convenience functions.
  \li KDPenSaver, KDBrushSaver, KDFontSaver, KDTransformMapper - more fine-grained versions of KDPainterSaver
  \li KDTransformMapper / KDMatrixMapper - Classes to turn QTransform / QMatrix into a functor
  \li KDMetaMethodIterator - Iterator over methods of QObjects
  \li KDSaveFile - Secure and robust writing to a file
  \li KDUIntProperty - A property with an unsigned integer value, for KDPropertyView
  \li KDMultipleChoiceProperty - A property with a multiple item selection
  \li KDSearchLineEdit - A line edit usually used to type in text to search
  \li KDLockButton - A Mac-style widget to indicate GUI elements are locked from/by the user
  \li KDListViewToolBar - Mac-style toolbar to be placed below list views
  \li KDHelpButton - A Mac-style help button
  \li KDDoubleSpinBox - A double spinbox with improved locale support
  \li KDCocoaUtils - Mac OS X Cocoa related convenience functions
  \li KDCircularProgressIndicator - A Mac-style circular (rotating) progress indicator
  \li KDUpdater::PackagesModel - A model that can show packages contained in KDUpdater::PackagesInfo

  \section newmethods23 New Member Functions

  \li KDPainterSaver::KDPainterSaver(QPainter&)
  \li pimpl_ptr::swap
  \li KDAutoPointer::operator T*()
  \li KDUnitTest::Test::expectNextCheckToFail()

  \section newproperties23 New Properties

  \section newmacros23 New Macros

  \li KDAB_HAVE_CPP11_EXTERN_TEMPLATES
  \li KDAB_HAVE_CPP11_EXPLICIT_VIRTUAL_OVERRIDE
  \li KDAB_OVERRIDE
  \li KDAB_FINAL
  \li KDAB_FINAL_CLASS

  \section changes23 Other Changes

  \li KDTools now compiles against a namespaced Qt, with GCC 4.7 and Clang, and with some feature-reduced Qt configurations
  \li KDUnitTest::Runner - Fix the bug that run( "foo" ) would not execute group "foo/bar" \bug{KDTO-259}
  \li KDUnitTest::Runner - Detect exceptions escaping from \link KDUnitTest::Test::run() Test::run()\endlink and report them as a failure.
  \li KDEmailValidator - Fix a couple of bugs where we prohibited inserting characters in the middle of an email address \bug{KDTO-194}\bug{KDTO-195}
  \li KDUpdater::Target - This class does no longer inherit QObject
  \li KDPropertyView - Fix a crash when model() == 0 \bug{KDTO-196}
  \li KDGenericFactory - Shares more code between instantiations.
  \li KDThreadRunner - Make it possible to call startThread() again after the thread terminated \bug{KDTO-178}; document that the thread implementation can call thread()->quit().
  \li KDTimeScrollBar - Actually reimplement keyPressEvent()
  \li Correct KDChoiceProperty position in display mode
  \li Correct checkbox position for KDBooleanProperty
  \li Fix update emit when a property is updated
  \li KDRect - Initial support for C++11 'constexpr'
*/

/*!
  \page whatsnew24 Classes, Functions, Properties, etc. new in KD Tools 2.4

  Links to new classes, functions, properties, and other items introduced in KD Tools 2.4.

  \li \link newlibraries24 New Libraries\endlink
  \li \link newclasses24 New Classes\endlink
  \li \link newmethods24 New Member Functions\endlink
  \li \link newproperties24 New Properties\endlink
  \li \link newmacros24 New Macros\endlink
  \li \link changes24 Other Changes\endlink

  \section newlibraries24 New Libraries

  \section newclasses24 New Classes

  \li KDIntPropertyEditor - A new property editor for int with maximum and minimum value support, for KDPropertyView
  \li KDDoublePropertyEditor - A new property editor for double with maximum and minimum value support, for KDPropertyView

  \section newmethods24 New Member Functions

  \section newproperties24 New Properties

  \section newmacros24 New Macros

  \section changes24 Other Changes
*/
