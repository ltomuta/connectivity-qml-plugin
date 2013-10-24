/**
 * Copyright (c) 2012 Nokia Corporation.
 * All rights reserved.
 *
 * For the applicable distribution terms see the license text file included in
 * the distribution.
 */

#include "wlanserver.h"

#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QRegExp>
#include <QHostInfo>
#include <QStringList>

#include "wlannetworkmgr.h"
#include "common.h"

//Constants
const int BroadCastInterval(5000); //In Milliseconds
const int BroadCastIntervalAfterFirstConnection(20000);

/*!
  \class WlanServer
  \brief Implements WLAN server that clients can connect to.
*/


/*!
  Constructor.
*/
WlanServer::WlanServer(QObject *parent) :
    QObject(parent),
    mDiscoveryServer(0),
    mTcpServer(0),
    mBroadcastSocket(0),
    mBroadcastPort(0),
    mState(QNetworkSession::Disconnected),
    mMaxConnections(0),
    mLastErrorString("")
{
    QString serverName("");
#if defined(Q_OS_SYMBIAN)
    serverName = QString("symbian-server");
#elif defined(Q_WS_HARMATTAN)
    serverName = QString("harmattan-server");
#else
    serverName = QString("qt-server");
#endif

    mBroadcastTimer.setInterval(BroadCastInterval);
    mBroadcastTimer.setSingleShot(false);
    connect(&mBroadcastTimer, SIGNAL(timeout()),
            this, SLOT(broadcastServerInfo()));

    mServerInfo.setHostname(serverName);
}

/*!
  Destructor.
*/
WlanServer::~WlanServer()
{
    stopServer();
}

QString WlanServer::clientName(int index) const
{
    if (index >= 0 && index < mSockets.size()) {
        return mSockets[index]->peerAddress().toString();
    }

    return "";
}

QString WlanServer::errorString() const
{
    return mLastErrorString;
}

/*!
    Creates the TCP-Server and starts to listen for incoming connections
    on \a port. Also starts broadcasting server information over UDP to \a bdport.
    Returns true if server was started succesfully, false otherwise.
*/
bool WlanServer::startServer(int port, int bdport)
{
    mLastErrorString = "";
    mBroadcastPort = bdport;
    mServerInfo.setPort(port);

    Common::resetBuffer();

    qDebug() << "WlanServer::startServer(): Serverport:" << mServerInfo.port()
             << "Broadcastport:" << mBroadcastPort;
    if (mTcpServer && mTcpServer->isListening()) {
        qDebug() << "WlanServer::startServer(): Will stop server.";
        stopServer();
    }

    //If we are connected to a network
    if (mState == QNetworkSession::Connected) {
        if (!mTcpServer) {
            mTcpServer = new QTcpServer(this);
        }

        if (!mDiscoveryServer) {
            mDiscoveryServer = new QTcpServer(this);
        }

        if (!mDiscoveryServer->isListening()) {
            int discoveryport = mServerInfo.port() +
                               (mBroadcastPort > mServerInfo.port() ? -1 : 1);

            if (mDiscoveryServer->listen(QHostAddress::Any, discoveryport)) {
                qDebug() << "WlanServer::startServer(): Listening for discoveries on"
                         << discoveryport;
                connect(mDiscoveryServer, SIGNAL(newConnection()),
                        this, SLOT(onNewDiscoveryConnection()));
            }
        }

        if (!mTcpServer->isListening()) {
            if (mTcpServer->listen(QHostAddress::Any, mServerInfo.port())) {
                connect(mTcpServer, SIGNAL(newConnection()),
                        this, SLOT(onNewConnection()));

                qDebug() << "WlanServer::startServer(): Server listening to port"
                         << mTcpServer->serverPort();

                if (!mBroadcastSocket) {
                    mBroadcastSocket = new QUdpSocket(this);
                }

                broadcastServerInfo();

                return true;
            } else {
                qDebug() << "WlanServer::startServer(): Listen failed.";
            }
        }
    } else {
        qDebug() << "WlanServer::startServer(): Pending start...";
        emit reconnectToNetwork();
    }

    return false;
}

/*!
  Terminates the connection, deletes the sockets, UDP socket and the TCP Server.
*/
void WlanServer::stopServer()
{
    qDebug() << "WlanServer::stopServer(): =>";

    foreach (QTcpSocket* socket, mSockets) {       
        socket->disconnectFromHost();
        delete socket;
        socket = 0;
    }

    //Delete server after all the sockets have been disconnected.
    if (mTcpServer) {
        mTcpServer->close();
        delete mTcpServer;
        mTcpServer = 0;
    }

    if (mDiscoveryServer) {
        mDiscoveryServer->close();
        delete mDiscoveryServer;
        mDiscoveryServer = 0;
    }

    mBroadcastTimer.stop();

    if (mBroadcastSocket) {
        mBroadcastSocket->close();
        delete mBroadcastSocket;
        mBroadcastSocket = 0;
    }

    qDebug() << "WlanServer::stopServer(): <=";
}

/*!
  Writes \a data to the open socket. Returns the number of last bytes written or -1
  if failed to write any data.
*/
qint64 WlanServer::write(const QByteArray &data)
{
    qint64 bytes = -1;
    foreach (QTcpSocket* socket, mSockets) {
        if ((bytes = socket->write(data)) < 0) {
            return -1;
        }
    }
    return bytes;
}


/*!
  Handles when server \a ip has changed.
*/
void WlanServer::onIpChanged(QString ip)
{
    qDebug() << "WlanServer::onIpChanged():" << ip;
    mServerInfo.setAddress(QHostAddress(ip));
}

/*!
  Handles when \a serverName has changed.
*/
void WlanServer::onServerNameChanged(QString serverName)
{    
    qDebug() << "WlanServer::onServerNameChanged():" << serverName;
    mServerInfo.setHostname(serverName);
}

/*!
  Handles when NetworkSession \a state has changed.
*/
void WlanServer::onNetworkStateChanged(QNetworkSession::State state)
{
    qDebug() << "WlanServer::onNetworkStateChanged():" << state;
    mState = state;
}

void WlanServer::onSocketError(QAbstractSocket::SocketError error)
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());

    if (socket) {
        mLastErrorString = socket->errorString();
    }

    emit socketError((int) error);
}

void WlanServer::onServerPortChanged(int port)
{
    qDebug() << "WlanServer::onServerPortChanged():" << port;
    mServerInfo.setPort(port);
}

void WlanServer::onBroadcastPortChanged(int port)
{
    qDebug() << "WlanServer::onBroadcastPortChanged():" << port;
    mBroadcastPort = port;
}

void WlanServer::setMaxConnections(int max)
{
    qDebug() << "WlanServer::setMaxConnections():" << max;
    mMaxConnections = max;
}

void WlanServer::setState(QNetworkSession::State state)
{
    qDebug() << "WlanServer::setState():" << state;
    mState = state;
}

void WlanServer::setIp(const QString &ip)
{
    qDebug() << "WlanServer::setIp():" << ip;
    mServerInfo.setAddress(QHostAddress(ip));
}

/*!
  Handles the incoming connection from the client. Connects required signals
  and slots of the new connection in order to receive data from the client.
*/
void WlanServer::onNewConnection()
{
    qDebug() << "WlanServer::onNewConnection()";

    QTcpSocket *socket = mTcpServer->nextPendingConnection();

    bool roomForMore = mMaxConnections > 0 && mSockets.size() < mMaxConnections;
    bool maxOk = mMaxConnections <= 0;
    bool hasPeer = hasPeerAddress(socket->peerAddress());

    if (!hasPeer && (maxOk || roomForMore)) {
        connect(socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
        connect(socket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));

        mSockets.append(socket);

        qDebug() << "WlanServer::onNewConnection(): Peer address:"
                 << socket->peerAddress().toString();

        //On connection, we'll start broadcasting less often
        mBroadcastTimer.setInterval(BroadCastIntervalAfterFirstConnection);

        emit clientConnected(socket->peerAddress().toString());
        return;
    }

    if (!roomForMore) {
        qDebug() << "WlanServer::onNewConnection(): Server is full.";
    } else if (hasPeer) {
        qDebug() << "WlanServer::onNewConnection():"
                 << "Client already connected!";

    }

    socket->close();
}

/*!
  Handles the disconnection of the client.
*/
void WlanServer::onDisconnected()
{
    qDebug() << "WlanServer::onDisconnected(): =>";
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());

    if (!socket) {
        qDebug() << "WlanServer::onDisconnected(): No socket.";
        return;
    }

    mSockets.removeOne(socket);
    socket->deleteLater();

    //If all clients have disconnected we'll start broadcasting more frequently
    if (mSockets.isEmpty()) {
        mBroadcastTimer.setInterval(BroadCastInterval);
    }

    emit clientDisconnected(mSockets.size());

    qDebug() << "WlanServer::onDisconnected(): <=";
}


/*!
  Receives data from the socket.
*/
void WlanServer::onReadyRead()
{
    if (mSockets.empty()) {
        return;
    }

    qDebug() << "WlanServer::onReadyRead(): =>";

    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());

    if (!socket) {
        qDebug() << "WlanServer::onReadyRead(): No socket.";
        return;
    }

    Common::readFromSocket(socket, this, "read", "WlanServer::onReadyRead():");

    qDebug() << "WlanServer::onReadyRead(): <=";
}

/*!
  Broadcasts the server information over UDP socket to broadcastport.
*/
void WlanServer::broadcastServerInfo()
{
    if (mBroadcastSocket) {
        QString bdMessage = QString("CONNPLUGIN %1").arg(mServerInfo.toString());
        qDebug() << "WlanServer::broadcastServerInfo(): Broadcasting:" << bdMessage << "to" << mBroadcastPort;

#ifndef Q_OS_SYMBIAN
        // On Harmattan/Linux based OSes IP stack sends frames only to the
        // first network adapter if packets are sent to 255.255.255.255 or
        // QHostAddress::Broadcast on AUTOIP net
        QRegExp rx("169\\.254\\.\\d{1,3}.\\d{1,3}");

        if (rx.exactMatch(mServerInfo.address().toString())) {
            qDebug() << "Exact match";
            mBroadcastSocket->writeDatagram(bdMessage.toAscii(),
                                            QHostAddress("169.254.255.255"),
                                            mBroadcastPort);
        }
        else {
            mBroadcastSocket->writeDatagram(bdMessage.toAscii(),
                                            QHostAddress::Broadcast,
                                            mBroadcastPort);
        }
#else
        mBroadcastSocket->writeDatagram(bdMessage.toAscii(),
                                        QHostAddress::Broadcast,
                                        mBroadcastPort);
#endif

        mBroadcastSocket->flush();

        if (!mBroadcastTimer.isActive()) {
            // Re-broadcast until a client starts interacting.
            mBroadcastTimer.start();
        }
    }
}

bool WlanServer::hasPeerAddress(const QHostAddress &address)
{
    foreach (QTcpSocket *socket, mSockets) {
        if (socket->peerAddress().toString() == address.toString()) {
            return true;
        }
    }

    return false;
}

void WlanServer::onNewDiscoveryConnection()
{
    qDebug() << "WlanServer::onNewDiscoveryConnection(): =>";

    QTcpSocket *socket = mTcpServer->nextPendingConnection();

    if (socket) {
        qDebug() << "WlanServer::onNewDiscoveryConnection(): Discovery client connected. Closing it now.";
        socket->close();
    }

    qDebug() << "WlanServer::onNewDiscoveryConnection(): <=";
}

