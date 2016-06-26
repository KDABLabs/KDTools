# This file was generated automatically.
# Please edit generate-configure.sh/autogen.py/ensure-stage-pri.sh rather than this file.
exists( $$PWD/../stage.pri ):include( $$PWD/../stage.pri )
for( __stage_pri_pro_file__, $$list($$files( *.pro )) ) {
    __stage_pri_pro_file__ ~= s/\\.pro$/.pri/
    exists( $$__stage_pri_pro_file__ ):include( $$__stage_pri_pro_file__ )
}
exists( $$PWD/local.pri ):include( $$PWD/local.pri )
