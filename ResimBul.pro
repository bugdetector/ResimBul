#-------------------------------------------------
#
# Project created by QtCreator 2017-04-16T00:27:11
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ResimBul
TEMPLATE = app


SOURCES += main.cpp\
        Resimbul.cpp

HEADERS  += Resimbul.h

FORMS    += Resimbul.ui

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += opencv
