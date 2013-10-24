/**
 * Copyright (c) 2012 Nokia Corporation.
 * All rights reserved.
 *
 * For the applicable distribution terms see the license text file included in
 * the distribution.
 */

#include "wlanconnection.h"

#include "wlanclient.h"
#include "wlanserver.h"
#include "wlandiscoverymgr.h"
#include "wlannetworkmgr.h"

#include <QDebug>

/*!
  \class WlanConnection
  \brief TODO
*/

/*!
  Constructor.
*/
WlanConnection::WlanConnection(QObject *parent)
    : ConnectionIf(parent),
      mServerPort(13001),
      mBroadcastPort(13002),
      mServer(0),
      mClient(0),
      mDiscoveryMgr(0)
{
    WlanNetworkMgr& mgr = Network::networkManager();

    QObject::connect(&mgr, SIGNAL(ipChanged(QString)),
                     this, SLOT(onIpChanged(QString)));

    QObject::connect(&mgr, SIGNAL(stateChanged(QNetworkSession::State)),
                     this, SLOT(onNetworkStateChanged(QNetworkSession::State)));

    mgr.connectToNetwork();
}


/*!
  Destructor.
*/
WlanConnection::~WlanConnection()
{
    disconnect();

    WlanNetworkMgr& mgr = Network::networkManager();
    mgr.disconnect();
}


/*!
  From ConnectionIf.
*/
void WlanConnection::setConnectAs(ConnectAs connectAs)
{
    qDebug() << "WlanConnection::setConnectAs():" << connectAs;
    ConnectionIf::setConnectAs(connectAs);

    onReconnect();

    if (connectAs == Server && !mServer) {
        delete mDiscoveryMgr;
        mDiscoveryMgr = 0;
        delete mClient;
        mClient = 0;      

        startServer();

    } else if (connectAs == Client && !mClient) {
        delete mServer;
        mServer = 0;

        startDiscoveryMgr();
        startClient();

    } else if (connectAs == DontCare) {
        startDiscoveryMgr();
        startServer();
        startClient();
    }

}


/*!
  From ConnectionIf.
*/
ConnectionIf::ConnectionType WlanConnection::type() const
{
    return LAN;
}

int WlanConnection::serverPort() const
{
    return mServerPort;
}

int WlanConnection::broadcastPort() const
{
    return mBroadcastPort;
}

void WlanConnection::setMaxConnections(int max)
{
    ConnectionIf::setMaxConnections(max);
    qDebug() << "WlanConnection::setMaxConnections():" << max;

    if (mServer) {
        mServer->setMaxConnections(max);
    }
}

/*!
  Starts connection.
*/
bool WlanConnection::connect()
{
    onReconnect();

    if (mServer) {
        WlanNetworkMgr& mgr = Network::networkManager();
        mServer->setMaxConnections(mMaxConnections);
        mServer->setState(mgr.state());
        mServer->setIp(mgr.ip());
    }

    if (mConnectAs == Server && mServer) {
        if (mServer->startServer(mServerPort, mBroadcastPort)) {            
            setStatus(Connecting);
            return true;
        }
    } else if (mConnectAs == Client && mDiscoveryMgr && mClient) {
        if (mDiscoveryMgr->startDiscovery(mBroadcastPort)) {
            setStatus(Discovering);
            return true;
        }
    } else if (mConnectAs == DontCare && mDiscoveryMgr && mClient && mServer) {
        bool serverOk = mServer->startServer(mServerPort, mBroadcastPort);
        bool discoveryOk = mDiscoveryMgr->startDiscovery(mBroadcastPort);

        if (serverOk && discoveryOk) {
            setStatus(Connecting);
            return true;
        }

    }
    return false;
}

/*!
  Connects to server given in \a info. This method should usually be called after
  services have been discovered and the user selects one to connect to.
  Returns true if successful, false otherwise.
*/
bool WlanConnection::connectToServer(const QString &info)
{
    if ((mConnectAs == Client || mConnectAs == DontCare) &&
         mDiscoveryMgr && mClient)
    {
        NetworkServerInfo serverInfo = mDiscoveryMgr->server(NetworkServerInfo(info));
        //If server hasn't been discovered, try to see if info contains valid server info
        if (!serverInfo.isValid()) {
            serverInfo = NetworkServerInfo(info);
            //If no port has been given, try using the default serverport
            if (serverInfo.port() == -1) {
                serverInfo.setPort(mServerPort);
            }
        }
        qDebug() << "WlanConnection::connectToServer():"
                 << serverInfo.toString();

        if (serverInfo.isValid()) {
            mClient->startClient(serverInfo);
            setStatus(Connecting);
            return true;
        }

    }
    return false;
}


/*!
  Disconnects.
*/
void WlanConnection::disconnect()
{
    qDebug() << "WlanConnection::disconnect(): =>";

    if (mClient) {
        mClient->stopClient();
    }

    if (mServer) {
        mServer->stopServer();
    }

    if (mDiscoveryMgr) {
        mDiscoveryMgr->stopDiscovery();
    }

    setStatus(NotConnected);


    qDebug() << "WlanConnection::disconnect(): <=";
}


/*!
  Sends \a message. Returns true if successful, false otherwise.
*/
bool WlanConnection::send(const QByteArray &message)
{
    if (mConnectAs == Client && mClient) {
        return mClient->write(message) > 0;
    }

    if (mConnectAs == Server && mServer) {
        return mServer->write(message) > 0;
    }

    if (mConnectAs == DontCare) {
        return (mClient ? mClient->write(message) > 0 : false) ||
               (mServer ? mServer->write(message) > 0 : false);
    }

    return false;
}

/*!
  Sets the serverport to \a port.
*/
void WlanConnection::setServerPort(int port)
{
    qDebug() << "WlanConnection::setServerPort(): Server port changing to" << port;
    mServerPort = port;
}

/*!
  Sets the broadcast port to \a port.
*/
void WlanConnection::setBroadcastPort(int port)
{
    qDebug() << "WlanConnection::setBroadcastPort(): Broadcast port changing to" << port;
    mBroadcastPort = port;
}

/*!
*/
void WlanConnection::onServerFound(NetworkServerInfo info)
{
    //If we are dontcare and we found a server, we'll stop our server and connect to it as client
    if (mConnectAs == DontCare && mStatus == Connecting) {
        setStatus(Discovering);
    }

    emit discovered(info.toString());
}

/*!
  This gets called when server exists. Used when ConnectAs is DontCare
  so we can handle changes between being a client or a server.
*/
void WlanConnection::onServerExists(NetworkServerInfo info)
{
    Q_UNUSED(info);
    if (mConnectAs == DontCare && mStatus == Connecting
        && mClient && !mClient->clientStarted())
    {
        setStatus(Discovering);
    }
}

/*!
  Called when server goes unavailable.
  \a index contains the index of the server.
*/
void WlanConnection::onServerRemoved(int index)
{
    qDebug() << "WlanConnection::onServerRemoved(): Removed" << index;
    emit removed(index);
}


/*!
  Forwards the read data.
*/
void WlanConnection::onRead(const QByteArray &data)
{
    qDebug() << "WlanConnection::onRead():" << data.size() << "bytes";
    emit received(QString(data));
}

/*!
  When we connect as a client to \a peer.
*/
void WlanConnection::onConnected(const QString &peer)
{
    //If we are connecting as DontCare we'll stop the server
    if (mConnectAs == DontCare && mServer) {
        mServer->stopServer();
    }

    if (mDiscoveryMgr) {
        mDiscoveryMgr->stopDiscovery();
    }

    mConnectedTo = peer;
    setStatus(Connected);
}

/*!
*/
void WlanConnection::onClientConnected(const QString &peer)
{
    //If we dont care wheter or not we are client or server and we get a client connected
    //We'll stop looking for servers and stay as a server until all clients have disconnected
    if (mConnectAs == DontCare) {
        mDiscoveryMgr->stopDiscovery();
        mClient->stopClient();
    }

    mConnectedTo = peer;
    setStatus(Connected);
}

/*!
  Gets called when a client disconnects from server.
*/
void WlanConnection::onClientDisconnected(int remainingClients)
{
    qDebug() << "WlanConnection::onClientDisconnected(): Remaining:" << remainingClients;

    //If we are DontCare and we have no more clients, we'll start discovery and client again
    if (mConnectAs == DontCare && remainingClients <= 0) {
       startDiscoveryMgr();
       startClient();
       mDiscoveryMgr->startDiscovery(mBroadcastPort);
    }

    if (mServer && remainingClients > 0) {
        mConnectedTo = mServer->clientName(remainingClients - 1);
    } else {
        mConnectedTo = "";
    }

    setStatus(Connecting);
}

/*!
  Gets called when client gets disconnected
*/
void WlanConnection::onDisconnected()
{
    qDebug() << "WlanConnection::onDisconnected(): =>";
    mConnectedTo = "";
    //If we are DontCare and we were the client, we'll restart the server
    if (mConnectAs == DontCare) {
        startServer();
        if (mServer->startServer(mServerPort, mBroadcastPort)) {
            setStatus(Connecting);
        }
    }

    if (mConnectAs != DontCare) {
        setStatus(NotConnected);
    }
    qDebug() << "WlanConnection::onDisconnected(): <=";
}

/*!
*/
void WlanConnection::onReconnect()
{
    WlanNetworkMgr& mgr = Network::networkManager();
    qDebug() << "WlanConnection::onReconnect(): =>" << mgr.state();
    if (mgr.state() != QNetworkSession::Connected &&
        mgr.state() != QNetworkSession::Connecting)
    {
        mgr.connectToNetwork();
    }
    qDebug() << "WlanConnection::onReconnect(): <=" << mgr.state();
}

void WlanConnection::onIpChanged(QString ip)
{
    mLocalName = ip;
}

void WlanConnection::onSocketError(int error)
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
  Creates and connects server and its signals
*/
void WlanConnection::startServer()
{
    if (!mServer) {
        mServer = new WlanServer(this);

        QObject::connect(mServer, SIGNAL(read(QByteArray)),
                         this, SLOT(onRead(QByteArray)));
        QObject::connect(mServer, SIGNAL(clientConnected(QString)),
                         this, SLOT(onClientConnected(QString)));
        QObject::connect(mServer, SIGNAL(clientDisconnected(int)),
                         this, SLOT(onClientDisconnected(int)));
        QObject::connect(mServer, SIGNAL(reconnectToNetwork()),
                         this, SLOT(onReconnect()));
        QObject::connect(mServer, SIGNAL(socketError(int)),
                         this, SLOT(onSocketError(int)));

        WlanNetworkMgr& mgr = Network::networkManager();
        QObject::connect(&mgr, SIGNAL(stateChanged(QNetworkSession::State)),
                         mServer, SLOT(onNetworkStateChanged(QNetworkSession::State)));
        QObject::connect(&mgr, SIGNAL(ipChanged(QString)),
                         mServer, SLOT(onIpChanged(QString)));

    }
}

/*!
  Creates and connects client and its signals
*/
void WlanConnection::startClient()
{
    if (!mClient) {
        mClient = new WlanClient(this);
        QObject::connect(mClient, SIGNAL(read(QByteArray)), this, SLOT(onRead(QByteArray)));
        QObject::connect(mClient, SIGNAL(connectedToServer(QString)), this, SLOT(onConnected(QString)));
        QObject::connect(mClient, SIGNAL(disconnectedFromServer()), this, SLOT(onDisconnected()));
        QObject::connect(mClient, SIGNAL(socketError(int)),
                         this, SLOT(onSocketError(int)));
    }
}

/*!
  Creates and connects the discovery manager and its signals.
*/
void WlanConnection::startDiscoveryMgr()
{
    if (!mDiscoveryMgr) {
        mDiscoveryMgr = new WlanDiscoveryMgr(this);
        QObject::connect(mDiscoveryMgr, SIGNAL(serverFound(NetworkServerInfo)),
                         this, SLOT(onServerFound(NetworkServerInfo)));
        QObject::connect(mDiscoveryMgr, SIGNAL(serverExists(NetworkServerInfo)),
                         this, SLOT(onServerExists(NetworkServerInfo)));
        QObject::connect(mDiscoveryMgr, SIGNAL(serverRemoved(int)),
                         this, SLOT(onServerRemoved(int)));
    }
}

void WlanConnection::onNetworkStateChanged(QNetworkSession::State state)
{
    switch (state) {
        case QNetworkSession::Invalid:
        case QNetworkSession::Disconnected:
            setNetworkStatus(NetworkNotConnected);
            break;
        case QNetworkSession::Connecting:
            setNetworkStatus(NetworkConnecting);
            break;
        case QNetworkSession::Connected:
            setNetworkStatus(NetworkConnected);
            break;
        case QNetworkSession::Closing:
            setNetworkStatus(NetworkDisconnecting);
            break;
        default:
            break;
    }
}
