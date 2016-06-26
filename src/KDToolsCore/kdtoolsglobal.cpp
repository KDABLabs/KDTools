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

#include "kdtoolsglobal.h"

#include <QByteArray>

#include <algorithm>

namespace {
    struct Version {
	int v[3];
    };

    static inline bool operator<( const Version & lhs, const Version & rhs ) {
	return std::lexicographical_compare( lhs.v, lhs.v + 3, rhs.v, rhs.v + 3 );
    }
    static inline bool operator==( const Version & lhs, const Version & rhs ) {
	return std::equal( lhs.v, lhs.v + 3, rhs.v );
    }
    KDTOOLS_MAKE_RELATION_OPERATORS( Version, static inline )
}

static Version kdParseQtVersion( const char * const version ) {
    if ( !version || qstrlen( version ) < 5 || version[1] != '.' || version[3] != '.' || ( version[5] != 0 && version[5] != '.' && version[5] != '-' ) )
	return Version(); // parse error
    const Version result = { { version[0] - '0', version[2] - '0', version[4] - '0' } };
    return result;
}

bool _kdCheckQtVersion_impl( int major, int minor, int patchlevel ) {
    static const Version actual = kdParseQtVersion( qVersion() ); // do this only once each run...
    const Version requested = { { major, minor, patchlevel } };
    return actual >= requested;
}

/*!
  \def KDAB_HAVE_CPP11_EXTERN_TEMPLATES
  \ingroup cpp11_features
  \since_d 2.3

  This macro is defined if the C++ compiler supports the "extern
  templates" feature of C++11.
*/

/*!
  \def KDAB_HAVE_CPP11_CONSTEXPR
  \ingroup cpp11_features
  \since_d 2.3

  This macro is defined if the C++ compiler supports the "constexpr"
  feature of C++11.
*/

/*!
  \def KDAB_HAVE_CPP11_EXPLICIT_VIRTUAL_OVERRIDE
  \ingroup cpp11_features
  \since_d 2.3

  This macro is defined if the C++ compiler supports the \c override
  and \c final keywords with full C++ semantics (incl. \c final on
  classes).

  Regardless of whether this macro is defined, the KDAB_OVERRIDE,
  KDAB_FINAL, and KDAB_FINAL_CLASS macros can be used (and may be
  functional even though this macro is not defined).
*/

/*!
  \def KDAB_OVERRIDE
  \ingroup cpp11_features
  \since_d 2.3

  This macro is always defined. If the compiler supports the C++11 \c
  override contextual keyword (or a sufficiently compatible
  non-standard extension exists), this is defined to \c override,
  otherwise it is defined to nothing.

  Examples:
  \code
  void virtualFunctionOne() const KDAB_OVERRIDE;
  Object * virtualFunctionTwo() const KDAB_OVERRIDE = 0;
  \endcode
  \sa KDAB_FINAL KDAB_FINAL_CLASS
*/

/*!
  \def KDAB_FINAL
  \since_d 2.3
  \ingroup cpp11_features

  This macro is always defined. If the compiler supports the C++11 \c
  final contextual keyword (or a sufficiently compatible non-standard
  extension exists), this is defined to \c final (or whatever the
  non-standard extension uses), otherwise it is defined to nothing.

  Examples:
  \code
  void virtualFunctionOne() const KDAB_FINAL;
  \endcode
  \sa KDAB_OVERRIDE KDAB_FINAL_CLASS
*/

/*!
  \def KDAB_FINAL_CLASS
  \ingroup cpp11_features
  \since_d 2.3

  This macro is always defined. If the compiler supports the C++11 \c
  final contextual keyword (or a sufficiently compatible non-standard
  extension exists) \em on \em classes, this is defined to \c final
  (or whatever the non-standard extension uses), otherwise it is
  defined to nothing.

  Examples:
  \code
  class Derived KDAB_FINAL_CLASS : public Base {
      // ...
  };
  \endcode
  \sa KDAB_FINAL KDAB_OVERRIDE
*/

/*!
  \def KDAB_DECL_CONSTEXPR
  \ingroup cpp11_features
  \since_d 2.3

  This macro is always defined. If the compiler supports the C++11 \c
  constexpr keyword (or a sufficiently compatible non-standard
  extension exists) \em on \em functions, this is defined to \c constexpr
  (or whatever the non-standard extension uses), otherwise it is
  defined to nothing.

  Examples:
  \code
  template <typename T>
  KDAB_DECL_CONSTEXPR T squared( const T & t ) { return t * t; }
  \endcode
  \sa KDAB_DECL_CONSTEXPR_VAR
*/

/*!
  \def KDAB_DECL_CONSTEXPR_VAR
  \ingroup cpp11_features
  \since_d 2.3

  This macro is always defined. If the compiler supports the C++11 \c
  constexpr keyword (or a sufficiently compatible non-standard
  extension exists) \em on \em data, this is defined to \c constexpr
  (or whatever the non-standard extension uses), otherwise it is
  defined to \c const.

  Examples:
  \code
  static KDAB_DECL_CONSTEXPR_VAR KDRect rects[] = {
    KDRect::fromPoints( 0, 0, 12, 12 );
    KDRect::fromPoints( 12, 12, 24, 24 );
  };
  \endcode
  \sa KDAB_DECL_CONSTEXPR
*/
