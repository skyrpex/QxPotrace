QT = core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TARGET = test
TEMPLATE = app

SOURCES += main.cpp

include(../QxPotrace.pri)
