/**
 * Copyright (c) 2012 Nokia Corporation.
 * All rights reserved.
 *
 * For the applicable distribution terms see the license text file included in
 * the distribution.
 */

#ifndef BLUETOOTHSTUBS_H
#define BLUETOOTHSTUBS_H

#include <QByteArray>
#include <QObject>
#include <QString>
#include <QVariant>
#include <QIODevice>

// Stub classes for Simulator
class QBluetoothLocalDevice : public QObject {
};

class QBluetoothAddress {
public:
    bool operator ==(const QBluetoothAddress&) const { return false; }
    QString toString() {
        return QString("StubAddress");
    }
};

class QBluetoothDeviceDiscoveryAgent : public QObject {
    Q_OBJECT
public:
    QBluetoothDeviceDiscoveryAgent(QObject *parent = 0)
        : QObject(parent) {}
    void start() {}
};

class QBluetoothUuid {
public:
    enum Enums {
        PublicBrowseGroup,
        Rfcomm
    };

    QBluetoothUuid() {}
    QBluetoothUuid(QLatin1String) {}
    QBluetoothUuid(quint32) {}
    QBluetoothUuid(Enums) {}
};

class QBluetoothDeviceInfo {
public:
    enum Enums {
        DataIncomplete,
        PhoneDevice
    };

    QBluetoothAddress address () const {
        return mAddress;
    }
    bool isValid() const { return false; }
    int majorDeviceClass() const { return 0; }
    QString name() const { return QString("StubName"); }
    void setServiceUuids(QList<QBluetoothUuid>, int) {}

    QBluetoothAddress mAddress;
};

class QBluetoothServiceInfo {
public:
    class Sequence {
    public:
        void clear();
        void operator <<(QVariant&) {}
    };

    enum AttributeId {
        ServiceName,
        ServiceProvider,
        ServiceDescription,
        ServiceRecordHandle,
        ServiceClassIds,
        BrowseGroupList,
        ProtocolDescriptorList
    };

    QBluetoothDeviceInfo device() const {
        return mDeviceInfo;
    }

    QVariant attribute(AttributeId) const { return QVariant(); }
    bool registerService() { return false; }
    //void setAttribute(AttributeId, QVariant) {}
    void setAttribute(AttributeId, Sequence&) {}
    void setAttribute(AttributeId, QBluetoothUuid) {}
    void setAttribute(AttributeId, uint) {}
    void setAttribute(AttributeId, QString) {}
    void setDevice(QBluetoothDeviceInfo) {}
    void setServiceAvailability(int) {}
    void setServiceUuid(QBluetoothUuid) {}
    void unregisterService() {}

    QBluetoothDeviceInfo mDeviceInfo;
};

class QBluetoothSocket : public QIODevice {
    Q_OBJECT

public:
    enum SocketError {
        Error = 0
    };

    enum SocketState {
        UnconnectedState
    };

    enum SocketType {
        UnknownSocketType = -1,
        L2capSocket = 0,
        RfcommSocket = 1
    };

    QBluetoothSocket(SocketType socketType = UnknownSocketType, QObject *parent = 0)
        : QIODevice(parent)
    {
        Q_UNUSED(socketType);
    }

    virtual qint64 readData(char *data, qint64 maxlen) {Q_UNUSED(data); Q_UNUSED(maxlen); return -1;}
    virtual qint64 readLineData(char *data, qint64 maxlen) {Q_UNUSED(data); Q_UNUSED(maxlen); return -1;}
    virtual qint64 writeData(const char *data, qint64 len) {Q_UNUSED(data); Q_UNUSED(len); return -1;}

    bool canReadLine() { return false; }
    void connectToService(const QBluetoothAddress &, quint16 port = 0) { Q_UNUSED(port); }
    void connectToService(const QBluetoothServiceInfo &) { }
    void close() {}
    void disconnectFromService() {}
    void abort() {}
    QString peerName() const { return "StubPeerName"; }
    QByteArray readLine() { return QByteArray(); }
    SocketState state() const { return UnconnectedState; }
    qint64 write(QByteArray) { return 0; }
};

class QRfcommServer : public QObject {
    Q_OBJECT
public:
    QRfcommServer(QObject *parent = 0)
        : QObject(parent) {}
    bool listen() { return false; }
    qint16 serverPort() const { return 0; }
    QBluetoothSocket *nextPendingConnection() { return 0; }

signals:
    void newConnection();
};

#endif // BLUETOOTHSTUBS_H
