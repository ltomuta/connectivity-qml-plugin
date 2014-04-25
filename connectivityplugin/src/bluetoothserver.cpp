/**
 * Copyright (c) 2012-2014 Microsoft Mobile.
 * All rights reserved.
 *
 * For the applicable distribution terms see the license text file included in
 * the distribution.
 */

#include "bluetoothserver.h"

#include <QDebug>
#include <QStringList>

#if defined(Q_WS_SIMULATOR) || defined(DISABLE_BLUETOOTH)
#include "bluetoothstubs.h"
#else
#include <qbluetoothsocket.h>
#include <QRfcommServer.h>
#endif

#include "common.h"

/*!
  \class BluetoothServer
  \brief Implementation of a bluetooth service.
*/

/*!
  Constructor.
*/
BluetoothServer::BluetoothServer(QObject *parent)
    : QObject(parent),
      mRfcommServer(0),
      mServiceUuid(0),
      mMaxConnections(0),
      mLastErrorString("")
{
}


/*!
  Destructor.
*/
BluetoothServer::~BluetoothServer()
{
    stopServer();
}

QString BluetoothServer::clientName(int index) const
{
    if (index >= 0 && index < mSockets.size()) {
        return mSockets[index]->peerName();
    }

    return "";
}

QString BluetoothServer::errorString() const
{
    return mLastErrorString;
}

/*!
  Sets the service information.
*/
void BluetoothServer::setServiceInfo(const QString &serviceName,
                                     const QString &serviceProvider,
                                     const QString &serviceDescription)
{
    qDebug() << "BluetoothServer::setServiceInfo():"
             << serviceName << serviceProvider;

    mServiceInfo.setAttribute(QBluetoothServiceInfo::ServiceName, serviceName);
    mServiceInfo.setAttribute(QBluetoothServiceInfo::ServiceProvider, serviceProvider);
    mServiceInfo.setAttribute(QBluetoothServiceInfo::ServiceDescription, serviceDescription);

    // Simple algorithm for generating the service UUID
    mServiceUuid = 0;

    for (int i = 0; i < serviceName.length(); ++i) {
        mServiceUuid += (int)serviceName.at(i).toAscii();
    }
    for (int i = 0; i < serviceProvider.length(); ++i) {
        mServiceUuid += (int)serviceProvider.at(i).toAscii();
    }

    qDebug() << "BluetoothServer::setServiceInfo(): Generated UUID:" << mServiceUuid;
}


/*!
  Creates the RFCOMM server and starts to listen for the incoming connections.
  Returns true if the server was started successfully, false otherwise.
*/
bool BluetoothServer::startServer()
{
    qDebug() << "Bluetoothserver::startServer(): =>";

    if (mServiceUuid == 0) {
        qDebug() << "BluetoothServer::startServer(): No service information set!";
        return false;
    }

    if (mRfcommServer) {
        qDebug() << "BluetoothServer::startServer(): Already started!";
        return false;
    }

    mSockets.clear();
    mLastErrorString = "";

    Common::resetBuffer();

    qDebug() << "Bluetoothserver::startServer(): Creating a server";
    // Create the server
    mRfcommServer = new QRfcommServer(this);
    connect(mRfcommServer, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
    qDebug() << "Bluetoothserver::startServer(): Server created";

#ifdef Q_WS_HARMATTAN
    if (!mRfcommServer->listen(QBluetoothAddress(), 11))
#else
    if (!mRfcommServer->listen())
#endif
    {
        qDebug() << "BluetoothServer::startServer():"
                 << "Error: mRfcommServer is not listening!";
        delete mRfcommServer;
        mRfcommServer = 0;
        return false;
    }

    qDebug() << "BluetoothServer::startServer(): Server is using port"
             << mRfcommServer->serverPort();

    mServiceInfo.setAttribute(QBluetoothServiceInfo::ServiceRecordHandle, (uint)0x00010010);

#if !defined(Q_WS_SIMULATOR) && !defined(DISABLE_BLUETOOTH)
    // Class Uuuid must contain at least one entry
    QBluetoothServiceInfo::Sequence classId;
    classId << QVariant::fromValue(QBluetoothUuid(mServiceUuid));
    mServiceInfo.setAttribute(QBluetoothServiceInfo::ServiceClassIds, classId);
#endif

    mServiceInfo.setServiceAvailability(1);

    qDebug() << "BluetoothServer::startServer(): Using service info:"
             << mServiceInfo.attribute(QBluetoothServiceInfo::ServiceName).toString()
             << mServiceInfo.attribute(QBluetoothServiceInfo::ServiceProvider).toString();

    // Set the Service UUID set
    mServiceInfo.setServiceUuid(QBluetoothUuid(mServiceUuid));

    // Set service discoverability
    mServiceInfo.setAttribute(QBluetoothServiceInfo::BrowseGroupList,
                              QBluetoothUuid(QBluetoothUuid::PublicBrowseGroup));

#if !defined(Q_WS_SIMULATOR) && !defined(DISABLE_BLUETOOTH)
    // Protocol descriptor list
    QBluetoothServiceInfo::Sequence protocolDescriptorList;
    QBluetoothServiceInfo::Sequence protocol;

    protocol.clear();
    protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::Rfcomm))
             << QVariant::fromValue(quint8(mRfcommServer->serverPort()));
    protocolDescriptorList.append(QVariant::fromValue(protocol));

    mServiceInfo.setAttribute(QBluetoothServiceInfo::ProtocolDescriptorList,
                              protocolDescriptorList);
#endif

    // Register the service
    if (mServiceInfo.registerService()) {
        qDebug() << "BluetoothServer::startServer():"
                 << "Service registered. Waiting for clients to connect.";
    }
    else {
        qDebug() << "BluetoothServer::startServer():"
                 << "Failed to register the service!";
        delete mRfcommServer;
        mRfcommServer = 0;
        return false;
    }


    qDebug() << "Bluetoothserver::startServer(): <=";


    return true;
}


/*!
  Terminates the connection, deletes the socket and the RFCOMM server.
*/
void BluetoothServer::stopServer()
{
    qDebug() << "BluetoothServer::stopServer(): =>";
    // Unregister the service
    mServiceInfo.unregisterService();


    qDebug() << "BluetoothServer::stopServer(): Connected sockets" << mSockets.size();

    // Close the sockets
    foreach (QBluetoothSocket* socket, mSockets) {                 
        qDebug() << "BluetoothServer::stopServer(): Deleting socket" << socket->peerName();
        socket->disconnectFromService();
        delete socket;
        socket = 0;
    }

    mSockets.clear();

    // Close the server
    delete mRfcommServer;
    mRfcommServer = 0;



    qDebug() << "BluetoothServer::stopServer(): <=";
}



/*!
  Writes \a data to the open socket. Returns the number of bytes written or -1
  if failed to write any data.
*/
qint64 BluetoothServer::write(const QByteArray &data)
{
    qint64 bytes = 0;

    foreach (QBluetoothSocket* socket, mSockets) {
        bytes = socket->write(data);
        if (bytes <= 0) {
            return -1;
        }
    }

    return bytes;
}

void BluetoothServer::setMaxConnections(int max)
{
    qDebug() << "BluetoothServer::setMaxConnections():" << max;
    mMaxConnections = max;
}


/*!
  Handles the incoming connection from the client. Connects required signals
  and slots of the new connection in order to receive data from the client.
*/
void BluetoothServer::onNewConnection()
{
    qDebug() << "BluetoothServer::onNewConnection(): =>";

    QBluetoothSocket *socket = mRfcommServer->nextPendingConnection();

    if (!socket) {
        qDebug() << "BluetoothServer::onNewConnection(): No Socket!";
        return;
    }

    bool roomForMore = mMaxConnections > 0 && mSockets.size() < mMaxConnections;
    bool maxOk = mMaxConnections <= 0;
    bool hasPeer = hasPeerName(socket->peerName());

    if (!hasPeer && (maxOk || roomForMore)) {
        connect(socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
        connect(socket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
        connect(socket, SIGNAL(error(QBluetoothSocket::SocketError)),
                this, SLOT(onSocketError(QBluetoothSocket::SocketError)));

        qDebug() << "BluetoothServer::onNewConnection(): Client connected:"
                 << socket->peerName();

        mSockets.append(socket);

        emit clientConnected(socket->peerName());

        qDebug() << "BluetoothServer::onNewConnection(): <=";
        return;
    }

    if (!roomForMore) {
        qDebug() << "BluetoothServer::onNewConnection(): Server is full.";
    } else if (hasPeer) {
        qDebug() << "BluetoothServer::onNewConnection():"
                 << "Client already connected!";
    }
    socket->abort();
    qDebug() << "BluetoothServer::onNewConnection(): <=";
}


/*!
  Handles the disconnection of the client.
*/
void BluetoothServer::onDisconnected()
{
    qDebug() << "BluetoothServer::onDisconnected(): =>";
    QBluetoothSocket *socket = qobject_cast<QBluetoothSocket*>(sender());

    if (!socket) {
        return;
    }

    mSockets.removeOne(socket);
    socket->deleteLater();
    emit clientDisconnected(mSockets.size());

    qDebug() << "BluetoothServer::onDisconnected(): <=";
}


/*!
  Receives data from the socket.
*/
void BluetoothServer::onReadyRead()
{
    qDebug() << "BluetoothServer::onReadyRead(): =>";
    QBluetoothSocket *socket = qobject_cast<QBluetoothSocket *>(sender());
    if (!socket) {
        qDebug() << "BluetoothServer::onReadyRead(): No socket.";
        return;
    }

    Common::readFromSocket(socket, this, "read",
                           "BluetoothServer::onReadyRead():");

    qDebug() << "BluetoothServer::onReadyRead(): <=";
}

bool BluetoothServer::hasPeerName(const QString &name)
{
    foreach (QBluetoothSocket* socket, mSockets) {
        if (socket && socket->peerName() == name) {
            return true;
        }
    }
    return false;
}

void BluetoothServer::onSocketError(QBluetoothSocket::SocketError error)
{
    QBluetoothSocket *socket = qobject_cast<QBluetoothSocket*>(sender());

    if (socket) {
        mLastErrorString = socket->errorString();
    }
    emit socketError((int) error);
}
