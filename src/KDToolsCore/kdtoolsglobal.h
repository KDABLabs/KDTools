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

#ifndef __KDTOOLS_KDTOOLSGLOBAL_H__
#define __KDTOOLS_KDTOOLSGLOBAL_H__

#include <QtCore/QtGlobal>

#define KDAB_DISABLE_COPY( x ) private: x( const x & ); x & operator=( const x & )

// QT_BEGIN_NAMESPACE and QT_END_NAMESPACE are not defined for older Qt versions, so define them here
#ifndef QT_BEGIN_NAMESPACE
#  define QT_BEGIN_NAMESPACE
#endif

#ifndef QT_END_NAMESPACE
#  define QT_END_NAMESPACE
#endif

#ifdef KDTOOLS_SHARED
#  ifdef BUILD_SHARED_KDTOOLSCORE
#    define KDTOOLSCORE_EXPORT Q_DECL_EXPORT
#  else
#    define KDTOOLSCORE_EXPORT Q_DECL_IMPORT
#  endif
#  ifdef BUILD_SHARED_KDTOOLSGUI
#    define KDTOOLSGUI_EXPORT Q_DECL_EXPORT
#  else
#    define KDTOOLSGUI_EXPORT Q_DECL_IMPORT
#  endif
#  ifdef BUILD_SHARED_KDTOOLSXML
#    define KDTOOLSXML_EXPORT Q_DECL_EXPORT
#  else
#    define KDTOOLSXML_EXPORT Q_DECL_IMPORT
#  endif
#  ifdef BUILD_SHARED_KDUPDATER
#    define KDTOOLS_UPDATER_EXPORT    Q_DECL_EXPORT
#  else
#    define KDTOOLS_UPDATER_EXPORT    Q_DECL_IMPORT
#  endif
#else // KDTOOLS_SHARED
#  define KDTOOLSCORE_EXPORT
#  define KDTOOLSGUI_EXPORT
#  define KDTOOLSXML_EXPORT
#  define KDTOOLS_UPDATER_EXPORT
#endif // KDTOOLS_SHARED

#define MAKEINCLUDES_EXPORT

#define DOXYGEN_PROPERTY( x )
#ifdef DOXYGEN_RUN
# define KDAB_IMPLEMENT_SAFE_BOOL_OPERATOR( func ) operator unspecified_bool_type() const { return func; }
# define KDAB_USING_SAFE_BOOL_OPERATOR( Class ) operator unspecified_bool_type() const;
#else
# define KDAB_IMPLEMENT_SAFE_BOOL_OPERATOR_IMPL( func, access )              \
    access: struct __safe_bool_dummy__ { void nonnull() {} };           \
        typedef void ( __safe_bool_dummy__::*unspecified_bool_type )(); \
    public:                                                             \
        operator unspecified_bool_type() const {                        \
            return ( func ) ? &__safe_bool_dummy__::nonnull : 0 ;       \
        }
# define KDAB_IMPLEMENT_SAFE_BOOL_OPERATOR( func ) \
    KDAB_IMPLEMENT_SAFE_BOOL_OPERATOR_IMPL( func, private )
# if defined(__clang__)
   // KDT3: use this one for all compilers
#  define KDAB_IMPLEMENT_SAFE_BOOL_OPERATOR_BASE( func )        \
    KDAB_IMPLEMENT_SAFE_BOOL_OPERATOR_IMPL( func, protected )
# else
   // KDT3: is this needed for binary compatibility on Windows?
#  define KDAB_IMPLEMENT_SAFE_BOOL_OPERATOR_BASE( func )        \
    KDAB_IMPLEMENT_SAFE_BOOL_OPERATOR_IMPL( func, private )
#endif
#define KDAB_USING_SAFE_BOOL_OPERATOR( Class ) \
    using Class::operator Class::unspecified_bool_type;
#endif

#define KDTOOLS_MAKE_RELATION_OPERATORS( Class, linkage )             \
    linkage bool operator>( const Class & lhs, const Class & rhs ) {  \
        return operator<( rhs, lhs );                                 \
    }                                                                 \
    linkage bool operator!=( const Class & lhs, const Class & rhs ) { \
        return !operator==( lhs, rhs );                               \
    }                                                                 \
    linkage bool operator<=( const Class & lhs, const Class & rhs ) { \
        return !operator>( lhs, rhs );                                \
    }                                                                 \
    linkage bool operator>=( const Class & lhs, const Class & rhs ) { \
        return !operator<( lhs, rhs );                                \
    }

template <typename T>
inline T & __kdtools__dereference_for_methodcall( T & o ) {
    return o;
}

template <typename T>
inline T & __kdtools__dereference_for_methodcall( T * o ) {
    return *o;
}

#define KDAB_SET_OBJECT_NAME( x ) __kdtools__dereference_for_methodcall( x ).setObjectName( QLatin1String( #x ) )

KDTOOLSCORE_EXPORT bool _kdCheckQtVersion_impl( int major, int minor=0, int patchlevel=0 );
static inline bool kdCheckQtVersion( unsigned int major, unsigned int minor=0, unsigned int patchlevel=0 ) {
    return (major<<16|minor<<8|patchlevel) <= static_cast<unsigned int>(QT_VERSION)
	|| _kdCheckQtVersion_impl( major, minor, patchlevel );
}

#define KDTOOLS_DECLARE_PRIVATE_BASE( Class )                        \
protected:                                                           \
    class Private;                                                   \
    Private * d_func() { return _d; }                                \
    const Private * d_func() const { return _d; }                    \
    Class( Private * _d_, bool b ) : _d( _d_ ) { init(b); }          \
private:                                                             \
    void init(bool);                                                 \
private:                                                             \
    Private * _d

#define KDTOOLS_DECLARE_PRIVATE_DERIVED( Class, Base )                  \
protected:                                                              \
    class Private;                                                      \
    Private * d_func() {                                                \
        return reinterpret_cast<Private*>( Base::d_func() );            \
    }                                                                   \
    const Private * d_func() const {                                    \
        return reinterpret_cast<const Private*>( Base::d_func() );      \
    }                                                                   \
    Class( Private * _d_, bool b )                                      \
        : Base( reinterpret_cast<Base::Private*>(_d_), b ) { init(b); } \
private:                                                                \
    void init(bool)

// C++11 feature macros:

#ifdef DOXYGEN_RUN
# define KDAB_HAVE_CPP11_EXTERN_TEMPLATES
# define KDAB_HAVE_CPP11_EXPLICIT_VIRTUAL_OVERRIDE
# define KDAB_HAVE_CPP11_CONSTEXPR
# define KDAB_OVERRIDE
# define KDAB_FINAL
# define KDAB_FINAL_CLASS
# define KDAB_DECL_CONSTEXPR
# define KDAB_DECL_CONSTEXPR_VAR const
#else

#ifdef __clang__ // Clang defines also __GNUC__, so this must be first

# if __has_feature(cxx_constexpr)
#  ifndef  KDAB_HAVE_CPP11_CONSTEXPR
#   define KDAB_HAVE_CPP11_CONSTEXPR
//#warning yup
#  endif
# endif // cxx_constexpr
# if __has_feature(cxx_override_control)
#  ifndef  KDAB_HAVE_CPP11_EXPLICIT_VIRTUAL_OVERRIDE
#   define KDAB_HAVE_CPP11_EXPLICIT_VIRTUAL_OVERRIDE
//#warning yup
#  endif
# endif // cxx_override_control
  // ### no __has_feature feature has been defined for external templates, yet (3.1-trunk)

#else // __clang__

# ifdef __GXX_EXPERIMENTAL_CXX0X__ // GCC-compatible
#  if __GNUC__ > 4 || __GNUC__ == 4 && __GNUC_MINOR__ >= 3  // GCC >= 4.3 w/-std=c++0x
#   ifndef  KDAB_HAVE_CPP11_EXTERN_TEMPLATES
#    define KDAB_HAVE_CPP11_EXTERN_TEMPLATES
#   endif
#  endif // GCC >= 4.3 w/-std=c++0x
#  if __GNUC__ > 4 || __GNUC__ == 4 && __GNUC_MINOR__ >= 6  // GCC >= 4.6 w/-std=c++0x
#   ifndef  KDAB_HAVE_CPP11_CONSTEXPR
#    define KDAB_HAVE_CPP11_CONSTEXPR
#   endif
#  endif // GCC >= 4.6 w/-std=c++0x
#  if __GNUC__ > 4 || __GNUC__ == 4 && __GNUC_MINOR__ >= 7  // GCC >= 4.7 w/-std=c++0x
#   ifndef  KDAB_HAVE_CPP11_EXPLICIT_VIRTUAL_OVERRIDE
#    define KDAB_HAVE_CPP11_EXPLICIT_VIRTUAL_OVERRIDE
#   endif
#  endif // GCC >= 4.7 w/-std=c++0x
# endif // __GXX_EXPERIMENTAL_CXX0X__

#endif // __clang__

#ifdef _MSC_VER // MSVC
# if _MSC_VER >= 1700 // >= VS2011
#  ifndef  KDAB_HAVE_CPP11_EXTERN_TEMPLATES
#   define KDAB_HAVE_CPP11_EXTERN_TEMPLATES // http://blogs.msdn.com/b/vcblog/archive/2011/09/12/10209291.aspx
#  endif
# endif
# if _MSC_VER == 1600 || _MSC_VER == 1700
#  ifndef  KDAB_HAVE_CPP11_EXPLICIT_VIRTUAL_OVERRIDE
//#   define KDAB_HAVE_CPP11_EXPLICIT_VIRTUAL_OVERRIDE // "partial" is not enough
#  endif
// define KDAB_{OVERRIDE,FINAL,FINAL_CLASS} directly, b/c KDAB_HAVE_CPP11_EXPLICIT_VIRTUAL_OVERRIDE isn't set:
#  ifndef KDAB_OVERRIDE
#   define KDAB_OVERRIDE override
#  endif
#  ifndef KDAB_FINAL
#   define KDAB_FINAL sealed // http://blogs.msdn.com/b/vcblog/archive/2011/09/12/10209291.aspx look for override/final
#  endif
#  ifndef KDAB_FINAL_CLASS
#   define KDAB_FINAL_CLASS // not supported?
#  endif
# endif
#endif

// check Qt macros:
#ifdef Q_COMPILER_CONSTEXPR
# ifndef KDAB_HAVE_CPP11_CONSTEXPR
#  define KDAB_HAVE_CPP11_CONSTEXPR
# endif
#endif
#ifdef Q_COMPILER_EXTERN_TEMPLATES
# ifndef KDAB_HAVE_CPP11_EXTERN_TEMPLATES
#  define KDAB_HAVE_CPP11_EXTERN_TEMPLATES
# endif
#endif

// set dependant macros:
#ifndef KDAB_OVERRIDE
# ifdef KDAB_HAVE_CPP11_EXPLICIT_VIRTUAL_OVERRIDE
#  define KDAB_OVERRIDE override
# else
#  define KDAB_OVERRIDE
# endif
#endif

#ifndef KDAB_FINAL
# ifdef KDAB_HAVE_CPP11_EXPLICIT_VIRTUAL_OVERRIDE
#  define KDAB_FINAL final
# else
#  define KDAB_FINAL
# endif
#endif

#ifndef KDAB_FINAL_CLASS
# ifdef KDAB_HAVE_CPP11_EXPLICIT_VIRTUAL_OVERRIDE
#  define KDAB_FINAL_CLASS KDAB_FINAL
# else
#  define KDAB_FINAL_CLASS
# endif
#endif

#ifndef KDAB_DECL_CONSTEXPR
# ifdef KDAB_HAVE_CPP11_CONSTEXPR
#  define KDAB_DECL_CONSTEXPR constexpr
# else
#  define KDAB_DECL_CONSTEXPR
# endif
#endif

#ifndef KDAB_DECL_CONSTEXPR_VAR
# ifdef KDAB_HAVE_CPP11_CONSTEXPR
#  define KDAB_DECL_CONSTEXPR_VAR constexpr
# else
#  define KDAB_DECL_CONSTEXPR_VAR const
# endif
#endif

#endif // DOXYGEN_RUN

// Internal min/max functions to work around std::min/max, qMin/Max
// (Qt<4.8) not being constexpr. There's apparently a bug in C++11
// where the intention was to allow returns by reference in constexpr
// functions (and GCC groks it) while the standardese says that's not
// allowed (and Clang implements it that way, and that's why
// std::min/max aren't constexpr). By returning by value, we should be
// able to avoid that issue here:

template <typename T>
inline KDAB_DECL_CONSTEXPR T kdMin( const T & lhs, const T & rhs ) {
    return !(rhs<lhs) ? lhs : rhs ;
}

template <typename T>
inline KDAB_DECL_CONSTEXPR T kdMax( const T & lhs, const T & rhs ) {
    return (lhs<rhs) ? rhs : lhs ;
}

#endif /* __KDTOOLS_KDTOOLSGLOBAL_H__ */

