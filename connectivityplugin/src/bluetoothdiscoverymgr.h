/**
 * Copyright (c) 2012 Nokia Corporation.
 * All rights reserved.
 *
 * For the applicable distribution terms see the license text file included in
 * the distribution.
 */

#ifndef BLUETOOTHDISCOVERYMGR_H
#define BLUETOOTHDISCOVERYMGR_H

#include <QMap>
#include <QObject>
#include <QString>
#include <QList>

#if defined(Q_WS_SIMULATOR) || defined(DISABLE_BLUETOOTH)
#include "bluetoothstubs.h"
#else
#include <qbluetoothdevicediscoveryagent.h>
#include <qbluetoothserviceinfo.h>
#include <qbluetoothuuid.h>

QTM_USE_NAMESPACE
#endif


class BluetoothDiscoveryMgr : public QObject
{
    Q_OBJECT

public:
    BluetoothDiscoveryMgr(QObject *parent = 0);
    ~BluetoothDiscoveryMgr();

public:
    Q_INVOKABLE QBluetoothServiceInfo service(int index) const;
    Q_INVOKABLE QBluetoothServiceInfo service(const QString &name) const;

public slots:
    bool startDiscovery(const QBluetoothUuid &uuid);
    void stopDiscovery();

private slots:
    void onDeviceDiscovered(const QBluetoothDeviceInfo &info);
    void onDiscoveryFinished();

signals:
    void deviceDiscovered(int index, const QString &deviceName);

private: // Data
    QBluetoothDeviceDiscoveryAgent *mDiscoveryAgent; // Owned
    QList<QBluetoothDeviceInfo> mDiscoveredDevices;
    QBluetoothUuid mUuid;
};

#endif // BLUETOOTHDISCOVERYMGR_H
