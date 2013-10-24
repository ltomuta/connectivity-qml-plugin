/**
 * Copyright (c) 2012 Nokia Corporation.
 * All rights reserved.
 *
 * For the applicable distribution terms see the license text file included in
 * the distribution.
 */

#include "bluetoothconnection.h"

#include <QDebug>

#include "bluetoothclient.h"
#include "bluetoothdiscoverymgr.h"
#include "bluetoothserver.h"

/*!
  \class BluetoothConnection
  \brief Implements a bluetooth connection.
*/

/*!
  Constructor.
*/
BluetoothConnection::BluetoothConnection(QObject *parent)
    : ConnectionIf(parent),
      mClient(0),
      mServer(0),
      mDiscoveryMgr(0),
      mServiceUuid(0),
      mLocalDevice(0)
{    
    //to obtain the local name of the device
#if !defined(Q_WS_SIMULATOR) && !defined(DISABLE_BLUETOOTH)
    mLocalDevice = new QBluetoothLocalDevice(this);
    mLocalName = mLocalDevice->name();

    QBluetoothLocalDevice::HostMode mode = mLocalDevice->hostMode();

    if (mode == QBluetoothLocalDevice::HostPoweredOff) {
        mLocalDevice->powerOn();
        mLocalDevice->setHostMode(QBluetoothLocalDevice::HostDiscoverable);
    }

    onBluetoothStateChanged(mLocalDevice->hostMode());

    QObject::connect(mLocalDevice,
                     SIGNAL(hostModeStateChanged(QBluetoothLocalDevice::HostMode)),
                     this,
                     SLOT(onBluetoothStateChanged(QBluetoothLocalDevice::HostMode)));
#endif
}


/*!
  Destructor.
*/
BluetoothConnection::~BluetoothConnection()
{
    disconnect();

    delete mClient;
    mClient = 0;

    delete mServer;
    mServer = 0;

    delete mDiscoveryMgr;
    mDiscoveryMgr = 0;

    delete mLocalDevice;
    mLocalDevice = 0;
}


/*!
  From ConnectionIf.
*/
void BluetoothConnection::setConnectAs(ConnectAs connectAs)
{
    qDebug() << "BluetoothConnection::setConnectAs():" << connectAs;
    ConnectionIf::setConnectAs(connectAs);

    if (connectAs == Server && !mServer) {
        delete mDiscoveryMgr;
        mDiscoveryMgr = 0;
        delete mClient;
        mClient = 0;

        startServer();
    }
    else if (connectAs == Client && !mClient) {
        delete mServer;
        mServer = 0;

        startDiscoveryMgr();
        startClient();
    }
}


/*!
  From ConnectionIf.
*/
void BluetoothConnection::setServiceInfo(const QString &serviceName,
                                         const QString &serviceProvider)
{
    ConnectionIf::setServiceInfo(serviceName, serviceProvider);

    if (mServer) {
        mServer->setServiceInfo(mServiceName, mServiceProvider);
    }

    // Simple algorithm for generating the service UUID
    mServiceUuid = 0;

    for (int i = 0; i < serviceName.length(); ++i) {
        mServiceUuid += (int)serviceName.at(i).toAscii();
    }
    for (int i = 0; i < serviceProvider.length(); ++i) {
        mServiceUuid += (int)serviceProvider.at(i).toAscii();
    }

    qDebug() << "BluetoothConnection::setServiceInfo(): Generated UUID:" << mServiceUuid;
}


/*!
  From ConnectionIf.
*/
ConnectionIf::ConnectionType BluetoothConnection::type() const
{
    return Bluetooth;
}

void BluetoothConnection::setMaxConnections(int max)
{
    ConnectionIf::setMaxConnections(max);
    qDebug() << "BluetoothConnection::setMaxConnections():" << max;
    if (mServer) {
        mServer->setMaxConnections(max);
    }
}

/*!
  Starts connection.
*/
bool BluetoothConnection::connect()
{
#if !defined(Q_WS_SIMULATOR) && !defined(DISABLE_BLUETOOTH)
    QBluetoothLocalDevice::HostMode mode = mLocalDevice->hostMode();

    if (mode == QBluetoothLocalDevice::HostPoweredOff) {
        qDebug() << "BluetoothConnection::connect(): Bluetooth is powered off";
        return false;
    }
#endif

    if (mConnectAs == Server && mServer) {
        mServer->setMaxConnections(mMaxConnections);
        if (mServer->startServer()) {
            setStatus(Connecting);
            return true;
        }
    }
    else if (mConnectAs == Client && mDiscoveryMgr && mClient) {
        if (mDiscoveryMgr->startDiscovery(QBluetoothUuid(mServiceUuid))) {
            setStatus(Discovering);
            return true;
        }
    }

    return false;
}


/*!
  Connects to service with \a name. This method should usually be called after
  services have been discovered and the user selects one to connect to.
  Returns true if successful, false otherwise.
*/
bool BluetoothConnection::connectToService(const QString &name)
{
    if (mConnectAs == Client && mDiscoveryMgr && mClient) {
        QBluetoothServiceInfo serviceInfo = mDiscoveryMgr->service(name);

        mDiscoveryMgr->stopDiscovery();
        mClient->startClient(serviceInfo);

        setStatus(Connecting);
        return true;
    }

    return false;
}


/*!
  Disconnects.
*/
void BluetoothConnection::disconnect()
{
    qDebug() << "BluetoothConnection::disconnect(): =>";
    if (mClient) {
        mClient->stopClient();
    }

    if (mServer) {
        mServer->stopServer();
    }

    if (mDiscoveryMgr) {
        mDiscoveryMgr->stopDiscovery();
    }

    mConnectedTo = "";
    setStatus(NotConnected);
    qDebug() << "BluetoothConnection::disconnect(): <=";
}


/*!
  Sends \a message. Returns true if successful, false otherwise.
*/
bool BluetoothConnection::send(const QByteArray &message)
{
    if (mClient) {
        return mClient->write(message) > 0;
    }

    if (mServer) {
        return mServer->write(message) > 0;
    }

    return false;
}


/*!
*/
void BluetoothConnection::onDeviceDiscovered(int index, const QString &name)
{
    Q_UNUSED(index);
    emit discovered(name);
}


/*!
*/
void BluetoothConnection::onConnected(const QString &name)
{
    if (mDiscoveryMgr) {
        mDiscoveryMgr->stopDiscovery();
    }
    mConnectedTo = name;
    setStatus(Connected);
}


/*!
  Called when client disconnects from server.
*/
void BluetoothConnection::onDisconnected()
{
    mConnectedTo = "";
    setStatus(NotConnected);
}

/*!
  Called when \c BluetoothServer emits \l BluetoothServer::clientDisconnected
*/
void BluetoothConnection::onClientDisconnected(int remainingClients)
{
    qDebug() << "BluetoothConnection::onClientDisconnected(): Remaining:" << remainingClients;

    if (mServer && remainingClients > 0) {
        mConnectedTo = mServer->clientName(remainingClients - 1);
    } else {
        mConnectedTo = "";
    }

    setStatus(Connecting);
}


/*!
  Forwards the read data.
*/
void BluetoothConnection::onRead(const QByteArray &data)
{
    qDebug() << "BluetoothConnection::onRead():" << data.size() << "bytes";
    emit received(QString(data));
}

void BluetoothConnection::onSocketError(int error)
{
    mError = error;
    mErrorString = "";

    if (mServer) {
        mErrorString = mServer->errorString();
    }

    if (mClient && mErrorString == "") {
        mErrorString = mClient->errorString();
    }

    emit errorOccured(error);
}

/*!
  Starts the client if one isn't started already.
*/
void BluetoothConnection::startClient()
{
    if (!mClient) {
        mClient = new BluetoothClient(this);
        QObject::connect(mClient, SIGNAL(connectedToService(QString)),
                         this, SLOT(onConnected(QString)));

        QObject::connect(mClient, SIGNAL(disconnectedFromServer()),
                         this, SLOT(onDisconnected()));

        QObject::connect(mClient, SIGNAL(read(QByteArray)),
                         this, SLOT(onRead(QByteArray)));

        QObject::connect(mClient, SIGNAL(socketError(int)),
                         this, SLOT(onSocketError(int)));
    }
}

/*!
  Starts the server if one isn't started already.
*/
void BluetoothConnection::startServer()
{
    if (!mServer) {
        mServer = new BluetoothServer(this);
        mServer->setServiceInfo(mServiceName, mServiceProvider);
        QObject::connect(mServer, SIGNAL(clientConnected(QString)),
                         this, SLOT(onConnected(QString)));

        QObject::connect(mServer, SIGNAL(clientDisconnected(int)),
                         this, SLOT(onClientDisconnected(int)));

        QObject::connect(mServer, SIGNAL(read(QByteArray)),
                         this, SLOT(onRead(QByteArray)));

        QObject::connect(mServer, SIGNAL(socketError(int)),
                         this, SLOT(onSocketError(int)));
    }
}

/*!
  Starts the discoverymanager if one isn't started already.
*/
void BluetoothConnection::startDiscoveryMgr()
{
    if (!mDiscoveryMgr) {
        mDiscoveryMgr = new BluetoothDiscoveryMgr(this);
        QObject::connect(mDiscoveryMgr, SIGNAL(deviceDiscovered(int,QString)),
                         this, SLOT(onDeviceDiscovered(int,QString)));
    }
}

#if !defined(Q_WS_SIMULATOR) && !defined(DISABLE_BLUETOOTH)
void BluetoothConnection::onBluetoothStateChanged(QBluetoothLocalDevice::HostMode state)
{
    switch (state) {
        case QBluetoothLocalDevice::HostPoweredOff:
            setNetworkStatus(NetworkNotConnected);
            break;
        case QBluetoothLocalDevice::HostConnectable:
        case QBluetoothLocalDevice::HostDiscoverable:
        case QBluetoothLocalDevice::HostDiscoverableLimitedInquiry:
            setNetworkStatus(NetworkConnected);
            break;
        default:
            break;
    }
}
#endif
