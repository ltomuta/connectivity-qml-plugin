/**
 * Copyright (c) 2012-2014 Microsoft Mobile.
 * All rights reserved.
 *
 * For the applicable distribution terms see the license text file included in
 * the distribution.
 */

#include "bluetoothdiscoverymgr.h"

#include <QDebug>
#include <QTimer>

#if !defined(Q_WS_SIMULATOR) && !defined(DISABLE_BLUETOOTH)
#include <qbluetoothdeviceinfo.h>
#include <qbluetoothaddress.h>

QTM_USE_NAMESPACE
#endif

// Constants
const int DiscoveryCycleInterval(1000); // Milliseconds


/*!
  \class BluetoothDiscoveryMgr
  \brief Handles the discovery of bluetooth services.
*/

/*!
  Constructor.
*/
BluetoothDiscoveryMgr::BluetoothDiscoveryMgr(QObject *parent)
    : QObject(parent),
      mDiscoveryAgent(0)
{
}


/*!
  Destructor.
*/
BluetoothDiscoveryMgr::~BluetoothDiscoveryMgr()
{
    delete mDiscoveryAgent;
}


/*!
  Getter for the QBluetoothServiceInfo of the discovered device by using
  \a index.
*/
QBluetoothServiceInfo BluetoothDiscoveryMgr::service(int index) const
{
    QList<QBluetoothUuid> uuidList;
    QBluetoothServiceInfo serviceInfo;

    if (index >= 0 && index < mDiscoveredDevices.size()) {
        uuidList.append(mUuid);
        serviceInfo.setDevice(mDiscoveredDevices[index]);
        serviceInfo.device().setServiceUuids(uuidList, QBluetoothDeviceInfo::DataIncomplete);
        serviceInfo.setServiceUuid(mUuid);
    }

    return serviceInfo;
}


/*!
  Getter for the QBluetoothServiceInfo of the discovered device by using
  \a name.
*/
QBluetoothServiceInfo BluetoothDiscoveryMgr::service(const QString &name) const
{
    QBluetoothDeviceInfo foundDeviceInfo;

    foreach (QBluetoothDeviceInfo info, mDiscoveredDevices) {
        if (info.name() == name) {
            foundDeviceInfo = info;
            break;
        }
    }

    if (!foundDeviceInfo.isValid()) {
        qDebug() << "BluetoothDiscoveryMgr::service():"
                 << "Failed to find device with name" << name;
        return QBluetoothServiceInfo();
    }

    QList<QBluetoothUuid> uuidList;
    QBluetoothServiceInfo serviceInfo;
    uuidList.append(mUuid);
    serviceInfo.setDevice(foundDeviceInfo);
    serviceInfo.device().setServiceUuids(uuidList, QBluetoothDeviceInfo::DataIncomplete);
    serviceInfo.setServiceUuid(mUuid);
    return serviceInfo;
}


/*!
  Starts the discovery of the Bluetooth devices in vicinity. Returns true if
  the discovery wes started successfully, false otherwise.
*/
bool BluetoothDiscoveryMgr::startDiscovery(const QBluetoothUuid &uuid)
{
    qDebug() << "BluetoothDiscoveryMgr::startDiscovery()";

    delete mDiscoveryAgent;
    mDiscoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);

#if !defined(Q_WS_HARMATTAN) && !defined(DISABLE_BLUETOOTH)
    connect(mDiscoveryAgent, SIGNAL(deviceDiscovered(const QBluetoothDeviceInfo&)),
            this, SLOT(onDeviceDiscovered(const QBluetoothDeviceInfo&)));
    connect(mDiscoveryAgent, SIGNAL(finished()),
            this, SLOT(onDiscoveryFinished()));
#endif

    mUuid = uuid;
    mDiscoveryAgent->start();
    return true;
}


/*!
  Stops the discovery.
*/
void BluetoothDiscoveryMgr::stopDiscovery()
{
    qDebug() << "BluetoothDiscoveryMgr::stopDiscovery()";
    delete mDiscoveryAgent;
    mDiscoveryAgent = 0;
}


/*!
  Appends the found device to the list of discovered devices and emits
  serverFound() signal.
*/
void BluetoothDiscoveryMgr::onDeviceDiscovered(const QBluetoothDeviceInfo &info)
{
    qDebug() << "BluetoothDiscoveryMgr::onDeviceDiscovered():"
             << info.name() << info.address().toString();

#ifndef SHOW_ALL_BT_DEVICES
    if (info.majorDeviceClass() != QBluetoothDeviceInfo::PhoneDevice) {
        qDebug() << "BluetoothDiscoveryMgr::onDeviceDiscovered():"
                 << "Not a phone, skipping.";
        return;
    }
#endif // SHOW_ALL_BT_DEVICES

    // Check if the device is already in
    foreach(QBluetoothDeviceInfo in, mDiscoveredDevices) {
        if (info.address() == in.address()) {
            qDebug() << "BluetoothDiscoveryMgr::onDeviceDiscovered():"
                     << "This device was discovered earlier.";
            return;
        }
    }


    int index = mDiscoveredDevices.size();
    mDiscoveredDevices.append(info);

    // Display the address of devices without name
    QString deviceName;

    if (info.name().isEmpty()) {
        deviceName = info.address().toString();
    } else {
        deviceName = info.name();
    }

    emit deviceDiscovered(index, deviceName);
}


/*!
  Restarts the discovery after a while.
*/
void BluetoothDiscoveryMgr::onDiscoveryFinished()
{
    qDebug() << "BluetoothDiscoveryMgr::onDiscoveryFinished():"
             << "Restarting in" << DiscoveryCycleInterval / 1000
             << "seconds.";
    QTimer::singleShot(DiscoveryCycleInterval, mDiscoveryAgent, SLOT(start()));    
}
