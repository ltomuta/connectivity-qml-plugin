# Copyright (c) 2012 Nokia Corporation.

QT += declarative
CONFIG += qt-components

TARGET = connectivityexample
TEMPLATE = app
VERSION = 0.1

SOURCES += src/main.cpp

RESOURCES += rsc/common.qrc

OTHER_FILES += \
        qml/common/*

# Speed up launching on MeeGo/Harmattan when using applauncherd daemon
# CONFIG += qdeclarative-boostable

include(../connectivityplugin/connectivityplugin.pri)

symbian {
    TARGET = ConnExample
    TARGET.UID3 = 0xE156DC86
    TARGET.CAPABILITY +=  NetworkServices \
                          LocalServices \
                          UserEnvironment \
                          ReadUserData \
                          WriteUserData \
    
    RESOURCES += rsc/symbian.qrc
    OTHER_FILES += qml/symbian/*

    ICON = icons/connectivityexample.svg
}

contains(MEEGO_EDITION,harmattan) {
    DEFINES += Q_WS_HARMATTAN

    contains(CONFIG,qdeclarative-boostable) {
        DEFINES += HARMATTAN_BOOSTER
    }

    RESOURCES += rsc/harmattan.qrc
    OTHER_FILES += qml/harmattan/*

    target.path = /opt/$${TARGET}/bin

    desktopfile.files = $${TARGET}.desktop
    desktopfile.path = /usr/share/applications
    icon.files += icons/connectivityexample.png
    icon.path = /usr/share/icons/hicolor/80x80/apps

    INSTALLS += \
        target \
        desktopfile \
        icon
}

simulator {
    RESOURCES += rsc/symbian.qrc
    OTHER_FILES += qml/symbian/*
}
