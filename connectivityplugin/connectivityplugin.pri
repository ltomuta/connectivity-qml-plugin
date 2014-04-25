# Copyright (c) 2012-2014 Microsoft Mobile.

QT += declarative network
CONFIG += qt plugin mobility
MOBILITY = connectivity

INCLUDEPATH += $$PWD/src

HEADERS += \
    $$PWD/src/bluetoothclient.h \
    $$PWD/src/bluetoothconnection.h \
    $$PWD/src/bluetoothdiscoverymgr.h \
    $$PWD/src/bluetoothserver.h \
    $$PWD/src/connectivityplugin.h \
    $$PWD/src/connectionif.h \
    $$PWD/src/connectionmanager.h \
    $$PWD/src/wlanconnection.h \
    $$PWD/src/wlanserver.h \
    $$PWD/src/wlanclient.h \
    $$PWD/src/wlandiscoverymgr.h \
    $$PWD/src/networkserverinfo.h \
    $$PWD/src/wlannetworkmgr.h \
    $$PWD/src/common.h

SOURCES += \
    $$PWD/src/bluetoothclient.cpp \
    $$PWD/src/bluetoothconnection.cpp \
    $$PWD/src/bluetoothdiscoverymgr.cpp \
    $$PWD/src/bluetoothserver.cpp \
    $$PWD/src/connectivityplugin.cpp \
    $$PWD/src/connectionif.cpp \
    $$PWD/src/connectionmanager.cpp \
    $$PWD/src/wlanconnection.cpp \
    $$PWD/src/wlanserver.cpp \
    $$PWD/src/wlanclient.cpp \
    $$PWD/src/wlandiscoverymgr.cpp \
    $$PWD/src/networkserverinfo.cpp \
    $$PWD/src/wlannetworkmgr.cpp \
    $$PWD/src/common.cpp

qmldir.files += $$PWD/src/qmldir
qmldir.path +=  $$[QT_INSTALL_IMPORTS]/$$TARGETPATH

INSTALLS += qmldir

simulator|win32|macx|unix:isEmpty(MEEGO_VERSION_MAJOR):!symbian {
    HEADERS += $$PWD/src/bluetoothstubs.h
    DEFINES += DISABLE_BLUETOOTH
}

