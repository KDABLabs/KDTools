*-g++*|*-clang*|*-llvm* {
        NORMAL_CFLAGS = -Wno-long-long
        !win32-g++:NORMAL_CFLAGS +=-ansi
        NORMAL_CXXFLAGS = \
	        -Wnon-virtual-dtor -Wundef -Wcast-align \
		-Wchar-subscripts -Wpointer-arith \
	        -Wwrite-strings -Wformat-security \
                -Wmissing-format-attribute

        gpp_version_raw_ = $$system( $$QMAKE_CXX -v 2>&1 )

        # -Wconversion gives too many warnings from Qt-4.4.3 with gcc-4.3.2 (was fine with gcc-4.2.4),
        # so only adding it for gcc < 4.3 (upgrade this rule when eventually  fixed in gcc)
        contains( gpp_version_raw_, 3\\.\\d\\.\\d|4\\.[0-2]\\.\\d ) {
                NORMAL_CXXFLAGS += -Wconversion
        }

        # Qt-4.2 has tools/designer/src/lib/uilib/ui4_p.h:263: error: comma at end of enumerator list
        !contains($$list($$[QT_VERSION]), 4.2.*) {
                NORMAL_CFLAGS += -pedantic
        }

        # Increase the debugging level from Qt's default
        CONFIG(debug, debug|release) {
                NORMAL_CXXFLAGS += -g3
                *-clang*:NORMAL_CXXFLAGS += -fcatch-undefined-behavior
        }

        !win32-g++:USABLE_CXXFLAGS = -Wold-style-cast # -Wshadow # TBD (Qt's headers are clean)
        HARD_CXXFLAGS = -Weffc++ -Wshadow
        PITA_CXXFLAGS = -Wunreachable-code

        QMAKE_CFLAGS_WARN_ON   += $$NORMAL_CFLAGS
        QMAKE_CXXFLAGS_WARN_ON += $$NORMAL_CFLAGS $$NORMAL_CXXFLAGS

        QMAKE_CXXFLAGS_WARN_ON += $$USABLE_CXXFLAGS
        #QMAKE_CXXFLAGS_WARN_ON += $$HARD_CXXFLAGS # headers must compile with this, code doesn't need to; needs patched Qt
        #QMAKE_CXXFLAGS_WARN_ON += $$PITA_CXXFLAGS # header would be nice, but it's probably pointless, due to noise from Qt and libstdc++

        exists( "$$PWD/.svn" )|exists( "$$PWD/../.git" ) { # enable -Werror only for developers
                QMAKE_CFLAGS_WARN_ON += -Werror
                QMAKE_CXXFLAGS_WARN_ON += -Werror
                # qendian.h uses C-style casts
                QMAKE_CXXFLAGS_WARN_ON += -Wno-error=old-style-cast
                # Clang complains about unused parameters in Qt headers (inline methods)
                *-clang*:QMAKE_CXXFLAGS_WARN_ON += -Wno-error=unused-parameter
        }

}
