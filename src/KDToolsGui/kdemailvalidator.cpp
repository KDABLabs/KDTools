/****************************************************************************
** Copyright (C) 2001-2016 Klar√§lvdalens Datakonsult AB, a KDAB Group company, info@kdab.com.
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

#include "kdemailvalidator.h"

#include <QRegExpValidator>
#include <QUrl>

#include <algorithm>
#include <cassert>

static bool is_valid( KDEmailValidator::Mode mode ) {
    switch ( mode ) {
    case KDEmailValidator::LocalPart:
    case KDEmailValidator::Domain:
    case KDEmailValidator::AddrSpec:
        return true;
    case KDEmailValidator::_NumModes:
        ;
    }
    return false;
}

static const char localpart_rx[] = "[a-zA-Z0-9!#$%&'*+/=?^_`{|}~-]+(?:\\.[a-zA-Z0-9!#$%&'*+/=?^_`{|}~-]+)*";
static const char domain_rx[] = "(?:[a-zA-Z0-9](?:[a-zA-Z0-9-]*[a-zA-Z0-9])?\\.)+[a-zA-Z0-9](?:[a-zA-Z0-9-]*[a-zA-Z0-9])?";

static inline QLatin1String defaultTLD() { return QLatin1String( "org" ); }

namespace {

    class ValidationStrategy {
    public:
        virtual ~ValidationStrategy() {}
        virtual QValidator::State validate( QString & s, int & pos ) const = 0;
#ifdef KDTOOLS_SUPPORTIDNA
        virtual void setAllowIdna( bool on, const QString & tld ) = 0;
#endif
    };

    class RegExpValidationStrategy : public ValidationStrategy {
        QRegExp rx;
    public:
        explicit RegExpValidationStrategy( const QRegExp & rx )
            : ValidationStrategy(), rx( rx ) {}

        QValidator::State validate( QString & input, int & pos ) const KDAB_OVERRIDE {
            // fixed version of QRegExpValidator::validate():
            if (rx.exactMatch(input)) {
                return QRegExpValidator::Acceptable;
            } else {
                if (const_cast<QRegExp &>(rx).matchedLength() >= /*input.size()*/pos) {
                    return QRegExpValidator::Intermediate;
                } else {
                    pos = input.size();
                    return QRegExpValidator::Invalid;
                }
            }
        }
    };

    class LocalPartValidationStrategy : public RegExpValidationStrategy {
    public:
        LocalPartValidationStrategy()
            : RegExpValidationStrategy( QRegExp( QLatin1String( localpart_rx ) ) ) {}

#ifdef KDTOOLS_SUPPORTIDNA
        /* reimp */ void setAllowIdna( bool, const QString & ) {}
#endif
    };

    class DomainValidationStrategy : public RegExpValidationStrategy {
        bool allowIdna;
        QString tld;
    public:
        DomainValidationStrategy()
            : RegExpValidationStrategy( QRegExp( QLatin1String( domain_rx ) ) ), allowIdna( true ), tld( defaultTLD() ) {}

#ifdef KDTOOLS_SUPPORTIDNA
        void setAllowIdna( bool on, const QString & tld_ ) KDAB_OVERRIDE { allowIdna = on; tld = tld_; }
#endif

        QValidator::State validate( QString & s, int & pos ) const KDAB_OVERRIDE {
            const QString savedS = s;
            const int savedPos = pos;
            QValidator::State result = validateInternal( s, pos );
            if ( result == QValidator::Invalid ) {
                // something's wrong - decide whether it's
                // intermediate or invalid by looking at the substring
                // [0,pos[:
                s = savedS.left( savedPos );
                pos = savedPos;
                result = std::min( validateInternal( s, pos ), QValidator::Intermediate );
                s += savedS.mid( savedPos );
            }
            return result;
        }

    private:
        QValidator::State validateInternal( QString & s, int & pos ) const {

#ifdef KDTOOLS_SUPPORTIDNA
            if ( !allowIdna )
                return RegExpValidationStrategy::validate( s, pos );

            // Interesting part:

            // The strategy here is to ace-encode the domain, then
            // check the result a) against
            // NonIdnaDomainValidationStrategy, and b) decodes to the
            // original again.

            // QUrl::toAce doesn't like intermediate domain names
            // (those that don't have a TLD with at least two
            // characters), as well as non-idn-whitelisted TLDs (such
            // as .com) so we need to provide for that, and later chop
            // them off again.

            static const QLatin1Char dot( '.' );

            const bool needsTldAdded = s.endsWith( dot );
            const bool needsDotTldAdded = !needsTldAdded;

            QString domain = s.toLower();
            if ( needsDotTldAdded )
                domain += QLatin1Char( '.' ) + tld;
            else if ( needsTldAdded )
                domain += tld;

            const QByteArray aceEncoded = QUrl::toAce( domain );
            const QString roundTripped = QUrl::fromAce( aceEncoded );

            if ( roundTripped != domain )
                return QValidator::Invalid;

            QString ace = QString::fromLatin1( aceEncoded.constData() );

            if ( needsDotTldAdded ) {
                assert( ace.endsWith( QLatin1Char( '.' ) + tld ) );
                ace.chop( 1 + tld.size() );
            } else if ( needsTldAdded ) {
                assert( ace.endsWith( QLatin1Char( '.' ) + tld ) );
                ace.chop( tld.size() ); // sic!
            }

            int dummy = ace.length();
            return RegExpValidationStrategy::validate( ace, dummy );
#else // KDTOOLS_SUPPORTIDNA
             return RegExpValidationStrategy::validate( s, pos );
#endif // KDTOOLS_SUPPORTIDNA
        }
    };

    class AddrSpecValidationStrategy : public ValidationStrategy {
        LocalPartValidationStrategy lv;
        DomainValidationStrategy dv;
    public:
        AddrSpecValidationStrategy()
            : ValidationStrategy(), lv(), dv() {}

#ifdef KDTOOLS_SUPPORTIDNA
        void setAllowIdna( bool on, const QString & tld ) KDAB_OVERRIDE { dv.setAllowIdna( on, tld ); }
#endif

        QValidator::State validate( QString & s, int & pos ) const KDAB_OVERRIDE {

            const int at = s.lastIndexOf( QLatin1Char( '@' ) );

            if ( at < 0 )
                // even if lv says it's Acceptable, we're still missing the '@'...
                return std::min( lv.validate( s, pos ), QValidator::Intermediate );

            QString localpart = s.left( at );
            QString domain = s.mid( at + 1 );

            int lvLen = localpart.size();
            int & lvPos = pos <= at ? pos : lvLen ;
            const QValidator::State lvResult = lv.validate( localpart, lvPos );
            if ( domain.isEmpty() )
                return std::min( lvResult, QValidator::Intermediate );

            int dvPos = pos > at ? pos - at - 1 : domain.size() ;
            const QValidator::State dvResult = dv.validate( domain, dvPos );
            if ( pos > at )
                pos = dvPos + at + 1;

            return std::min( lvResult, dvResult );
        }
    };

    template <typename Strategy>
    ValidationStrategy * create() { return new Strategy; }

} // anon namespace

typedef ValidationStrategy * (*create_fn)();

static const create_fn create_strategy[] = {
    &create<LocalPartValidationStrategy>,
    &create<DomainValidationStrategy>,
    &create<AddrSpecValidationStrategy>,
};

class KDEmailValidator::Private {
    friend class ::KDEmailValidator;
    KDEmailValidator * const q;
public:
    explicit Private( Mode m, KDEmailValidator * qq )
        : q( qq ),
          mode( m ),
#ifdef KDTOOLS_SUPPORTIDNA
          allowIdna( true ),
#endif
          tld( defaultTLD() ),
          cache()
    {
        /*static_*/assert( sizeof create_strategy / sizeof *create_strategy == _NumModes );
    }

    ~Private() {
        delete cache.strategy; cache.strategy = 0;
    }

private:
    void updateCache() const {
        if ( !cache.dirty )
            return;
        delete cache.strategy;
        cache.strategy = create_strategy[mode]();
        assert( cache.strategy );
#ifdef KDTOOLS_SUPPORTIDNA
        cache.strategy->setAllowIdna( allowIdna, tld );
#endif
        cache.dirty = false;
    }

private:
    Mode mode;
#ifdef KDTOOLS_SUPPORTIDNA
    bool allowIdna;
#endif
    QString tld;
    mutable struct Cache {
        Cache() : dirty( true ), strategy( 0 ) {}
        bool dirty;
        ValidationStrategy * strategy;
    } cache;
};

/*!
  \class KDEmailValidator
  \ingroup gui
  \since_c 2.2
  \brief A QValidator to validate email addresses and/or domain names

  This class can be used to validate email addresses in \link
  QLineEdit QLineEdits\endlink and everywhere else a QValidator can be
  used.

  Currently, KDEmailValidator can validate against these productions
  from <a href="http://www.ietf.org/rfc/rfc5322.txt">RFC 5322</a>:

  \li \c local-part the part of an email address before the \c @
  \li \c domain the domain part of the email address
  \li \c addr-spec This is what's commonly referred to as an email address: \c local-part \c @ \c domain

  Note that none of the obsolete \c obs-* productions and no unneeded
  CFWS (Comment-Folding-WhiteSpace) are allowed, contrary to the
  original productions from RFC 5322. Quoted \c local-part
  ("kdab†info"\@kdab.com) and literal domain names ([127.0.0.1]) also
  not permitted.

  These are the simplified productions this class supports:
  \verbatim
   addr-spec       =   local-part "@" domain

   local-part      =   dot-atom ;/ quoted-string / obs-local-part

   domain          =   dot-atom ;/ domain-literal / obs-domain

   dot-atom        =   dot-atom-text ;[CFWS] dot-atom-text [CFWS]

   dot-atom-text   =   1*atext *("." 1*atext)

   atom            =   [CFWS] 1*atext [CFWS]

   atext           =   ALPHA / DIGIT /    ; Printable US-ASCII
                       "!" / "#" /        ;  characters not including
                       "$" / "%" /        ;  specials.  Used for atoms.
                       "&" / "'" /
                       "*" / "+" /
                       "-" / "/" /
                       "=" / "?" /
                       "^" / "_" /
                       "`" / "{" /
                       "|" / "}" /
                       "~"
  \endverbatim

  In addition, \c domain is restricted to the characters
  <tt>[a-z0-9-.]</tt>, unless #allowInternationalizedDomainNames is \c
  true, in which case domain validity is determined by IDNA-processing
  inside Qt (QUrl::fromAce()/\link QUrl::toAce() toAce()\endlink).

  Other, more complex productions from RFC 5322 (\c address, \c
  mailbox) might be added in a future release, based on customer
  feedback.

  \note Validating IDNA-domains requires Qt >= 4.2.
*/

/*!
  \enum KDEmailValidator::Mode

  Determines which part of an email address to validate against.
*/

/*!
  \var KDEmailValidator::Mode KDEmailValidator::LocalPart
  validate according to the \c local-part production of RFC 5322
*/
/*!
  \var KDEmailValidator::Mode KDEmailValidator::Domain
  validate according to the \c domain production of RFC 5322
*/
/*!
  \var KDEmailValidator::Mode KDEmailValidator::AddrSpec
  validate according to the \c addr-spec production of RFC 5322
*/

/*!
  Constructor. Constructs a KDEmailValidator with parent \a parent and
  mode #AddrSpec.
*/
KDEmailValidator::KDEmailValidator( QObject * parent )
    : QValidator( parent ), d( new Private( AddrSpec, this ) )
{

}

/*!
  Constructor. Constructs a KDEmailValidator with parent \a parent and
  mode \a mode.
*/
KDEmailValidator::KDEmailValidator( Mode mode, QObject * parent )
    : QValidator( parent), d( new Private( mode, this ) )
{

}

/*!
  Destructor.
*/
KDEmailValidator::~KDEmailValidator() {}

/*!
  \property KDEmailValidator::mode

  Determines the part of an email address this validator is validating
  against. The default is #AddrSpec.

  Get this property's value using %mode(), and set it using setMode().
*/

/*!
  See #mode.
*/
void KDEmailValidator::setMode( Mode mode ) {
    if ( mode == d->mode )
        return;
    if ( !is_valid( mode ) ) {
        qWarning( "KDEmailValidator::setMode: invalid mode, ignoring" );
        return;
    }
    d->mode = mode;
    d->cache.dirty = true;
}

KDEmailValidator::Mode KDEmailValidator::mode() const {
    return d->mode;
}

/*!
  \property KDEmailValidator::allowInternationalizedDomainNames

  Specifies whether this validator should accept IDN's
  (Internationalized Domain Names, <a
  href="http://www.ietf.org/rfc/rfc3490.txt">RFC 3490</a>) in the
  domain part of the email address. The default is \c true.

  This property has no effect when #mode is #LocalPart.

  KDEmailValidator is a bit more permissive than QUrl::toAce() in that
  it allows IDNs under all TLDs (Top-Level-Domains), even when they
  have not (yet) been whiteisted for IDN use. E.g., as of this writing
  "klar‰lvdalens-datakonsult.se" validates as a domain according to
  QUrl, whereas "klar‰lvdalens-datakonsult.com" does not (the reason
  being that \c .com is currently not in the whitelist for IDN TLDs
  (Top-Level Domains). See also #idnEvaluationDomain.

  Get this property's value using %allowInternationalizedDomainNames(),
  and set it using setAllowInternationalizedDomainNames().
*/

#ifdef KDTOOLS_SUPPORTIDNA
/*!
  See #allowInternationalizedDomainNames.
*/
void KDEmailValidator::setAllowInternationalizedDomainNames( bool on ) {
    if ( on == d->allowIdna )
        return;
    d->allowIdna = on;
    if ( d->cache.strategy )
        d->cache.strategy->setAllowIdna( on, d->tld );
}

bool KDEmailValidator::allowInternationalizedDomainNames() const {
    return d->allowIdna;
}

/*!
  \property KDEmailValidator::idnEvaluationDomain

  Specifies the TLD (Top-Level-Domain) IDN domains are evaluated
  under. This is necessary for proper operation of validation, as
  described under #allowInternationalizedDomainNames. The default is
  "org", which should be good enough in the vast majority of cases.

  Get this property's value using %idnEvaluationDomain(), and set it
  using setIdnEvaluationDomain().
*/

/*!
  See #idnEvaluationDomain.
*/
void KDEmailValidator::setIdnEvaluationDomain( const QString & tld ) {
    if ( d->tld == tld )
        return;
    d->tld = tld;
    if ( d->cache.strategy )
        d->cache.strategy->setAllowIdna( d->allowIdna, tld );
}

QString KDEmailValidator::idnEvaluationDomain() const {
    return d->tld;
}
#endif

QValidator::State KDEmailValidator::validate( QString & str, int & pos ) const {

    d->updateCache();
    assert( d->cache.strategy );

    return d->cache.strategy->validate( str, pos );

}

#include "moc_kdemailvalidator.cpp"

#ifdef KDTOOLSGUI_UNITTESTS

#include <KDUnitTest/test.h>

namespace std {
    template <typename T, typename S>
    ostream & operator<<( ostream & s, const pair<T,S> & p ) {
        return s << p.first << p.second;
    }
}

KDAB_UNITTEST_SIMPLE( KDEmailValidator, "kdtools/gui" ) {

#define SET6( a, b, c, d, e, f ) { { QValidator::a, QValidator::b }, { QValidator::c, QValidator::d }, { QValidator::e, QValidator::f } }
#define SET3( x, y, z ) SET6( x, x, y, y, z, z )
#define SET2( u, v ) SET6( u, v, u, v, u, v )
#ifdef KDTOOLS_SUPPORTIDNA
 #define AS_IDNA( u ) SET6( Invalid, Invalid, Invalid, Invalid, Invalid, u )
 #define D_IDNA( u ) SET6( Invalid, Invalid, Invalid, u, Invalid, Invalid )
#else
 #define AS_IDNA( u ) SET6( Invalid, Invalid, Invalid, Invalid, Invalid, Invalid )
 #define D_IDNA( u ) SET6( Invalid, Invalid, Invalid, Invalid, Invalid, Invalid )
#endif
#define ALL( x ) SET2( x, x )

    static const struct _data {
        const char * input;
        QValidator::State expected[KDEmailValidator::_NumModes][2];
    } data[] = {
        { "M", SET3( Acceptable, Intermediate, Intermediate ) },
        { "m", SET3( Acceptable, Intermediate, Intermediate ) },
        { "klar\xC3\xA4",    D_IDNA( Intermediate ) },
        { "klar\xC3\xA4.",   D_IDNA( Intermediate ) },
        { "klar\xC3\xA4.s",  D_IDNA( Acceptable   ) }, // Intermediate?
        { "klar\xC3\xA4.se", D_IDNA( Acceptable   ) },
        { "klar\xC3\xA4.com",D_IDNA( Acceptable   ) }, // .com isn't idna-whitelisted in Qt, but we're working around that
        { "klar\xC3\xA4lvdalens-datakonsult.se", D_IDNA( Acceptable ) },
        { "info@",         SET3( Invalid, Invalid, Intermediate ) },
        { "info@k",        SET3( Invalid, Invalid, Intermediate ) },
        { "info@kd",       SET3( Invalid, Invalid, Intermediate ) },
        { "info@kda",      SET3( Invalid, Invalid, Intermediate ) },
        { "info@kdab",     SET3( Invalid, Invalid, Intermediate ) },
        { "info@kdab.",    SET3( Invalid, Invalid, Intermediate ) },
        { "info@kdab.n",   SET3( Invalid, Invalid, Acceptable   ) }, // Intermediate?
        { "info@kdab.ne",  SET3( Invalid, Invalid, Acceptable   ) },
        { "info@kdab.net", SET3( Invalid, Invalid, Acceptable   ) },
        { "info@klar\xC3\xA4",     AS_IDNA( Intermediate ) },
        { "info@klar\xC3\xA4l",    AS_IDNA( Intermediate ) },
        { "info@klar\xC3\xA4.",    AS_IDNA( Intermediate ) },
        { "info@klar\xC3\xA4.s",   AS_IDNA( Acceptable   ) },
        { "info@klar\xC3\xA4.se",  AS_IDNA( Acceptable   ) },
        { "info@klar\xC3\xA4..",   AS_IDNA( Invalid      ) },
    };

#undef ALL
#undef D_IDNA
#undef AS_IDNA
#undef SET2
#undef SET3
#undef SET6

#if 0
    KDEmailValidator validator( KDEmailValidator::AddrSpec );
    validator.setAllowInternationalizedDomainNames( true );
    QString s = QString::fromUtf8( data[sizeof data / sizeof *data - 1].input );
    int pos = s.size();
    assertEqual( validator.validate( s, pos ), data[sizeof data / sizeof *data - 1].expected[validator.mode()][validator.allowInternationalizedDomainNames()]);
#else
    for ( KDEmailValidator::Mode mode = KDEmailValidator::LocalPart ; mode < KDEmailValidator::_NumModes ; mode = static_cast<KDEmailValidator::Mode>( mode + 1 ) )
        for ( int /* sic! */ allowIdna = 0 ; allowIdna <= 1 ; ++allowIdna )
            for ( unsigned int i = 0 ; i < sizeof data / sizeof *data ; ++i ) {
                QString s = QString::fromUtf8( data[i].input );
                KDEmailValidator validator( mode );
#ifdef KDTOOLS_SUPPORTIDNA
                validator.setAllowInternationalizedDomainNames( allowIdna );
#endif
                int pos = s.size();
                assertEqual( std::make_pair( mode, validator.validate( s, pos ) ), std::make_pair( mode, data[i].expected[mode][allowIdna] ) );
            }
#endif

    // reproduce bugs here:
    {
        QString s;
        int pos;
        KDEmailValidator v( KDEmailValidator::AddrSpec );
#ifdef KDTOOLS_SUPPORTIDNA
        v.setAllowInternationalizedDomainNames( true );
#endif
        // verify bug KDTO-194:
        s = QLatin1String( "test.@kdab.com" );
        pos = 5;
        assertEqual( v.validate( s, pos ), QValidator::Intermediate );

        // verify bug KDTO-195:
        s = QLatin1String( "test.." );
        pos = 5;
        assertEqual( v.validate( s, pos ), QValidator::Intermediate );
        s = QLatin1String( "test@kdab..com" );
        pos =     qstrlen( "test@kdab." );
        assertEqual( v.validate( s, pos ), QValidator::Intermediate );
    }
}

#endif /* KDTOOLSGUI_UNITTESTS */
