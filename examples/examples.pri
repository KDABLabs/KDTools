CONFIG += kdtools
KDTOOLS += core gui

debug_and_release {
#	message( "debug and release" )
	CONFIG -= debug_and_release
	CONFIG += debug
}
