*-g++*|*-clang*|*-llvm* {

    # The QTest headers of Qt 4.7.4 cause a warning about strict aliasing,
    # so disable the warning here to be able to build with -Werror
    QMAKE_CFLAGS_WARN_ON += -Wno-strict-aliasing
    QMAKE_CXXFLAGS_WARN_ON += -Wno-strict-aliasing
}
