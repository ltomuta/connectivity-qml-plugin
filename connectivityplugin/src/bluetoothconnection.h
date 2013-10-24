/**
 * Copyright (c) 2012 Nokia Corporation.
 * All rights reserved.
 *
 * For the applicable distribution terms see the license text file included in
 * the distribution.
 */

#ifndef BLUETOOTHCONNECTION_H
#define BLUETOOTHCONNECTION_H

#include "connectionif.h"
#include <QObject>

#if !defined(Q_WS_SIMULATOR) && !defined(DISABLE_BLUETOOTH)
//#include <QBluetoothLocalDevice>
#include <qbluetoothlocaldevice.h>
QTM_USE_NAMESPACE
#else
class QBluetoothLocalDevice;
#endif

class BluetoothClient;
class BluetoothDiscoveryMgr;
class BluetoothServer;

class BluetoothConnection : public ConnectionIf
{
    Q_OBJECT

public:
    explicit BluetoothConnection(QObject *parent = 0);
    ~BluetoothConnection();

public:
    void setConnectAs(ConnectAs connectAs);
    void setServiceInfo(const QString &serviceName,
                        const QString &serviceProvider);
    ConnectionType type() const;

    void setMaxConnections(int max);

public slots:
    bool connect();
    bool connectToService(const QString &name);
    void disconnect();
    bool send(const QByteArray &message);

private slots:
    void onDeviceDiscovered(int index, const QString &name);
    void onConnected(const QString &name);
    void onDisconnected();
    void onClientDisconnected(int remainingClients);
    void onRead(const QByteArray &data);

    void onSocketError(int error);

    void startClient();
    void startServer();
    void startDiscoveryMgr();

#if !defined(Q_WS_SIMULATOR) && !defined(DISABLE_BLUETOOTH)
    void onBluetoothStateChanged(QBluetoothLocalDevice::HostMode state);
#endif


signals:
    void discovered(const QString &deviceName);

private: // Data
    BluetoothClient *mClient; // Owned
    BluetoothServer *mServer; // Owned
    BluetoothDiscoveryMgr *mDiscoveryMgr; // Owned
    quint32 mServiceUuid;
    QBluetoothLocalDevice* mLocalDevice;
};

#endif // BLUETOOTHCONNECTION_H
