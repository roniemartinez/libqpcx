#-------------------------------------------------
#
# Project created by QtCreator 2013-04-18T20:42:44
#
#-------------------------------------------------

QT       += core gui

CONFIG += plugin

CONFIG(debug, debug|release) {
    TARGET = qpcxd
}
CONFIG(release, debug|release) {
    TARGET = qpcx
}

TEMPLATE = lib

DESTDIR = $$[QT_INSTALL_PLUGINS]/imageformats

SOURCES += \
    qpcxhandler.cpp \
    qpcxplugin.cpp

HEADERS += \
    qpcxhandler.h \
    qpcxplugin.h
OTHER_FILES += \
    QPcxPlugin.json \
    LICENSE.LGPL

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
