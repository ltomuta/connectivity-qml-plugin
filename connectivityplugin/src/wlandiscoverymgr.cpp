/**
 * Copyright (c) 2012-2014 Microsoft Mobile.
 * All rights reserved.
 *
 * For the applicable distribution terms see the license text file included in
 * the distribution.
 */

#include "wlandiscoverymgr.h"

#include <QUdpSocket>
#include <QTcpSocket>
#include <QNetworkInterface>


//Constants
const int NumberOfTries(3);
const int TryInterval(20000); //Milliseconds

/*!
  \class WlanDiscoveryMgr
  \brief Handles the discovery of network servers.
*/

/*!
  Constructor.
*/
WlanDiscoveryMgr::WlanDiscoveryMgr(QObject *parent) :
    QObject(parent),
    mDiscoverySocket(0),
    mBroadcastTitle("CONNPLUGIN"),
    mBroadcastPort(0)
{
    //Construct a socket to listen for broadcasted server info
    mDiscoverySocket = new QUdpSocket(this);

    mServerCheckTimer.setInterval(TryInterval);
    mServerCheckTimer.setSingleShot(false);
    connect(&mServerCheckTimer, SIGNAL(timeout()),
            this, SLOT(checkServers()));
}

/*!
  Destructor.
*/
WlanDiscoveryMgr::~WlanDiscoveryMgr()
{
    stopDiscovery();
    delete mDiscoverySocket;
    mDiscoverySocket = 0;
}

/*!
  Getter for the NetworkServerInfo of the discovered server by using
  \a info. Info can be partial and it will match the closest.
*/
NetworkServerInfo WlanDiscoveryMgr::server(const NetworkServerInfo &info) const
{

    NetworkServerInfo foundInfo;
    foreach (NetworkServerInfo server, mDiscoveredServers) {
        bool hostOk = info.hostName() == server.hostName();
        bool addressOk = info.address() == server.address();
        bool portOk = info.port() == server.port();

        //If partial information was given we'll only match the parts that were given.
        if (info.hostName().isEmpty()) {
            hostOk = true;
        }

        if (info.address() == QHostAddress::Null) {
            addressOk = true;
        }

        if (info.port() == -1) {
            portOk = true;
        }

        if (hostOk && addressOk && portOk) {
            foundInfo = server;
            break;
        }
    }
    return foundInfo;
}

/*!
  Getter for the NetworkServerInfo of the discovered server by using
  \a name.
*/
NetworkServerInfo WlanDiscoveryMgr::server(const QString &hostName) const
{
    NetworkServerInfo foundInfo;
    foreach (NetworkServerInfo server, mDiscoveredServers) {
        if (hostName == server.hostName()) {
            foundInfo = server;
            break;
        }
    }
    return foundInfo;
}

/*!
  Getter for the NetworkServerInfo of the discovered server by using
  \a address.
*/
NetworkServerInfo WlanDiscoveryMgr::server(const QHostAddress &address) const
{
    NetworkServerInfo foundInfo;
    foreach (NetworkServerInfo server, mDiscoveredServers) {
        if (address == server.address()) {
            foundInfo = server;
            break;
        }
    }
    return foundInfo;
}

/*!
  Getter for the NetworkServerInfo of the discovered server by using
  \a port. Note: It will return the first matching server
  even in case multiple servers being available
*/
NetworkServerInfo WlanDiscoveryMgr::server(const int &port) const
{
    NetworkServerInfo foundInfo;
    foreach (NetworkServerInfo server, mDiscoveredServers) {
        if (port == server.port()) {
            foundInfo = server;
            break;
        }
    }
    return foundInfo;
}

/*!
  Starts the discovery of servers by listening to broadcasts on \a port.
  Returns true if the discovery wes started successfully, false otherwise.
*/
bool WlanDiscoveryMgr::startDiscovery(int port)
{
    qDebug() << "WlanDiscoveryMgr::startDiscovery():"
             << "Stopping any existing discoveries";
    stopDiscovery();

    qDebug() << "WlanDiscoveryMgr::startDiscovery():"
             << "Listening to server broadcast on port"
             << port;

    mBroadcastPort = port;
    mDiscoverySocket->bind(port, QUdpSocket::ShareAddress);
    connect(mDiscoverySocket, SIGNAL(readyRead()),
            this, SLOT(readBroadcast()));

    mServerCheckTimer.start();

    return true;
}

/*!
  Stops the discovery.
*/
void WlanDiscoveryMgr::stopDiscovery()
{
    qDebug() << "WlanDiscoveryMgr::stopDiscovery()";

    if (mDiscoverySocket) {
        mDiscoverySocket->close();
    }

    mBroadcastPort = 0;

    mServerCheckTimer.stop();
}

/*!
  Reads broadcast on the socket. If broadcast contains the server information
  we'll add the server to the list of discovered servers and we emit serverFound() signal.
*/
void WlanDiscoveryMgr::readBroadcast()
{
    qDebug() << "WlanDiscoveryMgr::readBroadcast()";
    while (mDiscoverySocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(mDiscoverySocket->pendingDatagramSize());
        mDiscoverySocket->readDatagram(datagram.data(), datagram.size());
        qDebug() << "WlanDiscoveryMgr::readBroadcast(): Read:" << datagram;

        //Broadcast should be in form of "CONNPLUGIN servername:ip:port"
        if (datagram.startsWith(mBroadcastTitle)) {

            //After the title rest of the datagram should contain server information.
            NetworkServerInfo info(datagram.mid(mBroadcastTitle.size() + 1));

            if (!QNetworkInterface::allAddresses().contains(info.address())) {
                foreach (NetworkServerInfo server, mDiscoveredServers) {
                    if (server.address() == info.address()) {
                        qDebug() << "WlanDiscoveryMgr::readBroadcast():"
                                 << "Server was discovered earlier";
                        emit serverExists(server);
                        return;
                    }
                }

                mDiscoveredServers.append(info);

                emit serverFound(info);
            }
        }
    }
}

/*!
  We'll check the servers periodically to make sure they are still active and that we
  are able to connect, if we are unable to connect we'll remove it from our discovered servers.
*/
void WlanDiscoveryMgr::checkServers()
{
    qDebug() << "WlanDiscoveryMgr::checkServers(): =>";
    QList<int> toRemove;

    int index = 0;
    foreach (NetworkServerInfo server, mDiscoveredServers) {

        int discoveryport = server.port() + (mBroadcastPort > server.port() ? -1 : 1);
        qDebug() << "WlanDiscoveryMgr::checkServers(): Checking connection to server"
                 << server.address().toString() + ":" + QString::number(server.port());

        QTcpSocket socket;

        socket.connectToHost(server.address(), discoveryport);

        //Wait for 5 seconds to connect
        if (!socket.waitForConnected(5000)) {
            qDebug() << "WlanDiscoveryMgr::checkServers(): Unable to connect.";
            toRemove.append(index);
        } else {
            qDebug() << "WlanDiscoveryMgr::checkServers(): Connected. Disconnecting.";
        }

        socket.disconnectFromHost();

        ++index;
    }

    //Servers will be removed in the reverse order that they were added in
    index = toRemove.size() - 1;
    for (; index >= 0; --index) {
        int idx = toRemove.at(index);
        mDiscoveredServers.removeAt(idx);
        emit serverRemoved(idx);
    }

    qDebug() << "WlanDiscoveryMgr::checkServers(): <=";
}
