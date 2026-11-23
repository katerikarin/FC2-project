QT += testlib
CONFIG += console testcase
CONFIG -= app_bundle
VPATH += "$$PWD/.."
INCLUDEPATH += "$$PWD/.."
include(../RuinsOfTheUnown.pro)
SOURCES -= main.cpp
SOURCES += animation.cpp
TARGET = animation
