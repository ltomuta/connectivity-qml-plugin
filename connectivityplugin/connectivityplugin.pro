# Copyright (c) 2012-2014 Microsoft Mobile.

QT += declarative network
CONFIG += qt plugin mobility
MOBILITY = connectivity

INCLUDEPATH += src

HEADERS += \
    src/bluetoothclient.h \
    src/bluetoothconnection.h \
    src/bluetoothdiscoverymgr.h \
    src/bluetoothserver.h \
    src/connectivityplugin.h \
    src/connectionif.h \
    src/connectionmanager.h \
    src/wlanconnection.h \
    src/wlanserver.h \
    src/wlanclient.h \
    src/wlandiscoverymgr.h \
    src/networkserverinfo.h \
    src/wlannetworkmgr.h \
    src/common.h

SOURCES += \
    src/bluetoothclient.cpp \
    src/bluetoothconnection.cpp \
    src/bluetoothdiscoverymgr.cpp \
    src/bluetoothserver.cpp \
    src/connectivityplugin.cpp \
    src/connectionif.cpp \
    src/connectionmanager.cpp \
    src/wlanconnection.cpp \
    src/wlanserver.cpp \
    src/wlanclient.cpp \
    src/wlandiscoverymgr.cpp \
    src/networkserverinfo.cpp \
    src/wlannetworkmgr.cpp \
    src/common.cpp

qmldir.files += src/qmldir
qmldir.path +=  $$[QT_INSTALL_IMPORTS]/$$TARGETPATH

INSTALLS += qmldir

simulator|win32|macx|unix:isEmpty(MEEGO_VERSION_MAJOR):!symbian {
    HEADERS += src/bluetoothstubs.h
    DEFINES += DISABLE_BLUETOOTH
}