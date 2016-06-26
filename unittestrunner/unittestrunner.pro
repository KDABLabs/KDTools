TEMPLATE = app

TARGET = unittestrunner

include( ../stage.pri )

CONFIG += console kdtools
CONFIG -= app_bundle
KDTOOLS += gui core

SOURCES += unittestrunner.cpp

include(../features/kdtools.prf)
