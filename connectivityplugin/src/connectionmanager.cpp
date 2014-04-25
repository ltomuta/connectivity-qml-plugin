/**
 * Copyright (c) 2012-2014 Microsoft Mobile.
 * All rights reserved.
 *
 * For the applicable distribution terms see the license text file included in
 * the distribution.
 */


#include "connectionmanager.h"

#include <QDebug>

#include "bluetoothconnection.h"
#include "wlanconnection.h"

#include "common.h"
#include "wlannetworkmgr.h"

/*!
  \enum ConnectionManager::ConnectionStatus
  \value NotConnected   Not connected to any service.
  \value Discovering    Discovering services.
  \value Connecting     Establishing a connection to a service.
  \value Connected      Connected to a service.
  \value Disconnecting  Connection is in the process of shutting down.
*/

/*!
  \enum ConnectionManager::ConnectionType
  \value Bluetooth  Using Bluetooth connection.
  \value LAN        Using WLAN connection.
*/

/*!
  \enum ConnectionManager::ConnectAs
  \value Client     Connecting as a client to a server.
  \value Server     Starting a service for clients to connect to
  \value DontCare   Used only for LAN connection. Starting both client to search for existing
                    services and a service to wait for clients to connect.
*/

/*!
  \property ConnectionManager::status
  This property holds the status of the connection.
*/

/*!
  \property ConnectionManager::connectionType
  This property holds the type of the connection.
  Default is \a Bluetooth.
*/

/*!
  \property ConnectionManager::connectAs
  This property holds the type of service used.

  Default is \a Server.

  For \a LAN connection \a DontCare option is provided.
*/

/*!
  \property ConnectionManager::connectionTimeout
  This property holds the time until the attempt to connect is terminated if not succesful.
*/

/*!
  \property ConnectionManager::serviceName
  This property holds the servicename.
  Defaults to \a ConnectivityPlugin
*/

/*!
  \property ConnectionManager::serviceProvider
  This property holds the serviceprovider.
  Defaults to \a Nokia.
*/

/*!
  \property ConnectionManager::peerName
  This property holds the name of the connected peer.

  When \a connectionType is \a Client this will hold the name of the server.
  When \a connectionType is \a Server this will hold the name of the last connected client.
*/

/*!
  \property ConnectionManager::error
  This property holds the last error that occured.
*/

/*!
  \property ConnectionManager::errorString
  This property holds the last error in a readable format.
*/

/*!
  \property ConnectionManager::localName
  This property holds the name of the owned connection.
  For \a Bluetooth default is the name of the device.
  For \a LAN default is the ip-address of the device.
*/

/*!
  \property ConnectionManager::serverPort
  This property holds the port used to accept connections from.
  Only used with \a LAN connection.

  Default is \a 13001.
*/

/*!
  \property ConnectionManager::broadcastPort
  This property holds the port used for broadcasting server information.
  Only used with \a LAN connection.

  Default is \a 13002.
*/

/*!
  \fn void ConnectionManager::disconnected()
  Connection was lost either by manually disconnecting or when the connected peer becomes unavailable.
*/

/*!
  \fn void ConnectionManager::received(const QString &message)
  A \a message was recieved.
*/

/*!
  \fn void ConnectionManager::discovered(const QString &name)
  A service was discovered. Service information is given in \a name.
*/

/*!
  \fn void ConnectionManager::removed(int index)
  A service has been closed with \a index.
  Used only for LAN connection so non-existant servers are not kept.
*/

// Constants
const QString DefaultServiceName("ConnectivityPlugin");
const QString DefaultServiceProvider("Nokia");


/*!
  \class ConnectionManager
  \brief TODO
*/


/*!
  Constructor.
*/
ConnectionManager::ConnectionManager(QObject *parent)
    : QObject(parent),
      mConnection(0),
      mServiceName(DefaultServiceName),
      mServiceProvider(DefaultServiceProvider),
      mStatus(NotConnected),
      mNetworkStatus(NetworkNotConnected),
#if !defined(DISABLE_BLUETOOTH)
      mConnectionType(Bluetooth),
#else
      mConnectionType(LAN),
#endif
      mConnectAs(ConnectionIf::Server),
      mConnectionTimeout(0),
      mMaxConnections(0),
      mServerPort(13001),
      mBroadcastPort(13002)
{
    mTimeoutTimer.setSingleShot(true);
    QObject::connect(&mTimeoutTimer, SIGNAL(timeout()), this, SLOT(disconnect()));
}

/*!
  Destructor.
*/
ConnectionManager::~ConnectionManager()
{
    disconnect();
    //Release the network manager,
    //Workaround for issue where NetworkManager didn't get deleted until
    //after the application eventloop was already over.
    Network::release();
}

/*!
  Returns the connection status.
*/
int ConnectionManager::status() const
{
    return mStatus;
}

int ConnectionManager::networkStatus() const
{
    return mNetworkStatus;
}

/*!
  Returns the connection type.
*/
int ConnectionManager::connectionType() const
{
    return mConnectionType;
}

/*!
  Sets the connection type to \a type.
*/
void ConnectionManager::setConnectionType(int type)
{
    qDebug() << "ConnectionManager::setConnectionType(): =>";
    if (mConnectionType == type && mConnection) {
        return;
    }

    disconnect();

    switch (type) {
    case Bluetooth:
        delete mConnection;
        mConnection = new BluetoothConnection(this);
        mConnectionType = Bluetooth;
        emit connectionTypeChanged(mConnectionType);
        break;
    case LAN:
        delete mConnection;
        mConnection = new WlanConnection(this);
        mConnectionType = LAN;
        emit connectionTypeChanged(mConnectionType);
        break;
    default:
        qDebug() << "ConnectionManager::setConnectionType(): Invalid type!";
    }
    mConnection->setMaxConnections(mMaxConnections);
    mConnection->setConnectAs(mConnectAs);
    mNetworkStatus = (ConnectionManager::NetworkStatus)mConnection->networkStatus();

    QObject::connect(mConnection, SIGNAL(statusChanged(ConnectionStatus)),
                     this, SLOT(onConnectionIfStatusChanged(ConnectionStatus)));

    QObject::connect(mConnection, SIGNAL(networkStatusChanged(NetworkStatus)),
                     this, SLOT(setNetworkStatus(NetworkStatus)));

    QObject::connect(mConnection, SIGNAL(received(QString)),
                     this, SIGNAL(received(QString)));

    QObject::connect(mConnection, SIGNAL(discovered(QString)),
                     this, SIGNAL(discovered(QString)));

    QObject::connect(mConnection, SIGNAL(errorOccured(int)),
                     this, SIGNAL(errorChanged(int)));


    if (mConnection->type() == ConnectionIf::LAN) {
        QObject::connect(mConnection, SIGNAL(removed(int)),
                         this, SIGNAL(removed(int)));      

        WlanConnection *lanConn = qobject_cast<WlanConnection*>(mConnection);
        if (lanConn) {
            lanConn->setBroadcastPort(mBroadcastPort);
            lanConn->setServerPort(mServerPort);
        }
    }

    qDebug() << "ConnectionManager::setConnectionType(): <=";
}

/*!
  Returns the connect as setting.
*/
int ConnectionManager::connectAs() const
{
    return mConnectAs;
}

/*!
  Sets the connect as setting.
*/
void ConnectionManager::setConnectAs(int connectAs)
{
    if (mStatus != NotConnected) {
        if (mConnection) {
            mConnection->disconnect();
            setStatus(NotConnected);
        }
    }

    if (mStatus == NotConnected) {
        switch (connectAs) {
        case ConnectionIf::Client:
            mConnectAs = ConnectionIf::Client;
            break;
        case ConnectionIf::Server:
            mConnectAs = ConnectionIf::Server;
            break;
        default:
            mConnectAs = ConnectionIf::DontCare;
            break;
        }

        if (mConnection) {
            mConnection->setConnectAs(mConnectAs);
        }

        emit connectAsChanged(mConnectAs);
    }
}

/*!
  Returns the connection timeout (in seconds).
*/
int ConnectionManager::connectionTimeout() const
{
    return mConnectionTimeout;
}

/*!
  Sets the connection timeout (in seconds). Setting timeout to 0 will disable
  timing out.
*/
void ConnectionManager::setConnectionTimeout(int timeout)
{
    mConnectionTimeout = timeout;
}

/*!
  Returns the service name.
*/
QString ConnectionManager::serviceName() const
{
    return mServiceName;
}

/*!
  Sets the service name.
*/
void ConnectionManager::setServiceName(const QString &name)
{
    mServiceName = name;
    emit serviceNameChanged(mServiceName);
}

/*!
  Returns the service provider.
*/
QString ConnectionManager::serviceProvider() const
{
    return mServiceProvider;
}

/*!
  Sets the service provider.
*/
void ConnectionManager::setServiceProvider(const QString &provider)
{
    mServiceProvider = provider;
    emit serviceProviderChanged(mServiceProvider);
}

/*!
  Returns the peer name.
*/
QString ConnectionManager::peerName() const
{
    return mPeerName;
}

/*!
  Returns the latest error value.
*/
int ConnectionManager::error() const
{
    if (mConnection) {
        return mConnection->error();
    }

    return 0;
}

/*!
  Returns the latest error in human readable form.
*/
QString ConnectionManager::errorString() const
{
    if (mConnection) {
        return mConnection->errorString();
    }

    return QString("No error");
}

/*!
  Returns the localname.
  For \c BluetoothConnection returns the name of the device.
  For \c WlanConnection returns the address of the device.
*/
QString ConnectionManager::localName() const
{
    if (mConnection) {
        return mConnection->localName();
    }

    return QString("Error: No connection");
}

/*!
  If \a connectionType is \a LAN this returns the port of the server.
  Otherwise returns -1;
*/
int ConnectionManager::serverPort() const
{
    if (mConnection && mConnection->type() == ConnectionIf::LAN) {
        WlanConnection *lanConn = qobject_cast<WlanConnection*>(mConnection);
        return lanConn->serverPort();
    }

    return -1;
}

/*!
  If \a connectionType is \a LAN this returns the port used for broadcasting.
  Otherwise returns -1;
*/
int ConnectionManager::broadcastPort() const
{
    if (mConnection && mConnection->type() == ConnectionIf::LAN) {
        WlanConnection *lanConn = qobject_cast<WlanConnection*>(mConnection);
        return lanConn->broadcastPort();
    }

    return -1;
}

int ConnectionManager::maxConnections() const
{
    return mMaxConnections;
}

/*!
  Sets serverport to \a port.
*/
void ConnectionManager::setServerPort(int port)
{
    mServerPort = port;
    if (mConnection && mConnection->type() == ConnectionIf::LAN) {
        WlanConnection *lanConn = qobject_cast<WlanConnection*>(mConnection);
        lanConn->setServerPort(mServerPort);
        emit serverPortChanged(mServerPort);
    }
}

/*!
  Sets broadcast port to \a port.
*/
void ConnectionManager::setBroadcastPort(int port)
{
    mBroadcastPort = port;
    if (mConnection && mConnection->type() == ConnectionIf::LAN) {
        WlanConnection *lanConn = qobject_cast<WlanConnection*>(mConnection);
        lanConn->setBroadcastPort(mBroadcastPort);
        emit broadcastPortChanged(mBroadcastPort);
    }
}

void ConnectionManager::setMaxConnections(int max)
{
    mMaxConnections = max;
    if (mConnection) {
        mConnection->setMaxConnections(mMaxConnections);
    }
    emit maxConnectionsChanged(mMaxConnections);
}

/*!
  Starts connection. If \a to is given tries to connect to it.
*/
void ConnectionManager::connect(const QString &to)
{
    qDebug() << "ConnectionManager::connect()";

    //If we have not established a connection and to is given,
    //try to establish a connection and then proceed
    if (mStatus == NotConnected && !to.isEmpty()) {
        if (!mConnection) {
            setConnectionType(mConnectionType);
        }

        applySettings();

        if (!mConnection) {
            qDebug() << "ConnectionManager::connect(): No Connection.";
        }

        if (!mConnection || !mConnection->connect()) {
            qDebug() << "ConnectionManager::connect(): Failed to connect!";
        }
    }

    bool isClient = mConnectAs == ConnectionIf::Client;
    bool isDontCare = mConnectAs == ConnectionIf::DontCare;
    bool isDiscovering = mStatus == Discovering;
    bool isLan = mConnectionType == LAN;
    bool isBluetooth = mConnectionType == Bluetooth;

    if (mStatus == NotConnected) {
        if (!mConnection) {
            setConnectionType(mConnectionType);
        }

        applySettings();

        if (!mConnection) {
            qDebug() << "ConnectionManager::connect(): No Connection.";
        }

        if (!mConnection || !mConnection->connect()) {
            qDebug() << "ConnectionManager::connect(): Failed to connect!";
        }
    } else if (isDiscovering && isBluetooth && isClient && !to.isEmpty()) {
        // Bluetooth client trying to connect to a discovered service/device
        qDebug() << "ConnectionManager::connect(): Will try to connect to" << to;

        if (mConnection && mConnection->type() == ConnectionIf::Bluetooth) {
            BluetoothConnection *btConn = qobject_cast<BluetoothConnection*>(mConnection);

            if (!btConn->connectToService(to)) {
                qDebug() << "ConnectionManager::connect():"
                         << "Failed to connect to" << to;
                disconnect();
            }
        }
    } else if (isDiscovering && isLan && (isClient || isDontCare) && !to.isEmpty()) {
        qDebug() << "ConnectionManager::connect(): Will try to connect to" << to;

        if (mConnection && mConnection->type() == ConnectionIf::LAN) {
            WlanConnection *lanConn = qobject_cast<WlanConnection*>(mConnection);

            if (!lanConn->connectToServer(to)) {
                qDebug() << "ConnectionManager::connect():"
                         << "Failed to connect to" << to;
                disconnect();
            }
        }

    } else {
        qDebug() << "ConnectionManager::connect():"
                 << "Invalid status for connecting!"
                 << "Try calling disconnect() first.";
    }
}

/*!
  Disconnects. If \a message is non-empty tries to broadcast it.
*/
void ConnectionManager::disconnect(const QString &message)
{
    qDebug() << "ConnectionManager::disconnect():" << message;

    if (!message.isEmpty()) {
        send(message);
    }

    if (mConnection) {
        mConnection->disconnect();
    }

    mPeerName = "";
    emit peerNameChanged(mPeerName);
    setStatus(NotConnected);
}

/*!
  Sends a \a message using the connection.
  If \a header is enabled we add a header to the data that describes the size.
  If \a compression is enabled data is compressed using default zlib compression.
  Returns true if successful, false otherwise.
*/
bool ConnectionManager::send(const QString &message, bool header /*= true*/, bool compression /*= false*/)
{
    if (mStatus == Connected && mConnection) {
        QByteArray msg;
        if (header) {
            msg = Common::toMessage(message, compression);
        } else {
            if (compression) {
                msg = qCompress(message.toAscii());
            } else {
                msg = message.toAscii();
            }
        }

        if (compression) {
            qDebug() << "ConnectionManager::send(): Original size:" << message.size();
        }
        qDebug() << "ConnectionManager::send(): Message size:" << msg.size();

        return mConnection->send(msg);
    }

    return false;
}

/*!
  Propagates the current settings to connection instance.
*/
void ConnectionManager::applySettings()
{
    if (mConnection) {
        mConnection->setServiceInfo(mServiceName, mServiceProvider);
    }
}

/*!
  Sets the status to \a status and emits a signal.
*/
void ConnectionManager::setStatus(ConnectionStatus status)
{
    if (mStatus != status) {
        mStatus = status;
        emit statusChanged(mStatus);
    }
}

/*!
  Sets the networkstatus to \a status and emits a signal.
*/
void ConnectionManager::setNetworkStatus(NetworkStatus status)
{
    if (status != mNetworkStatus) {
        mNetworkStatus = status;
        emit networkStatusChanged(status);
    }
}



/*!
*/
void ConnectionManager::onConnectionIfStatusChanged(ConnectionStatus status)
{
    qDebug() << "ConnectionManager::onConnectionIfStatusChanged():" << status;

    switch (status) {
    case ConnectionIf::NotConnected:
        if (mStatus == Connected) {
            // The connection has been severed and not by the user.
            emit disconnected();
        }

        mPeerName = "";
        emit peerNameChanged(mPeerName);
        setStatus(NotConnected);
        break;
    case ConnectionIf::Discovering:
        setStatus(Discovering);
        break;
    case ConnectionIf::Connecting:
        if (mConnectionTimeout > 0) {
            mTimeoutTimer.stop();
            mTimeoutTimer.setInterval(mConnectionTimeout * 1000);
            mTimeoutTimer.start();
        }        

        if (mConnectAs == ConnectionIf::Server ||
            mConnectAs == ConnectionIf::DontCare)
        {
            mPeerName = mConnection->connectedTo();
            emit peerNameChanged(mPeerName);
        }

        setStatus(Connecting);
        break;
    case ConnectionIf::Connected:
        if (mTimeoutTimer.isActive()) {
            mTimeoutTimer.stop();
        }

        mPeerName = mConnection->connectedTo();
        emit peerNameChanged(mPeerName);
        setStatus(Connected);
        break;
    case ConnectionIf::Disconnecting:
        setStatus(Disconnecting);
        break;
    }
}

