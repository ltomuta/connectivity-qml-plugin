/**
 * Copyright (c) 2012 Nokia Corporation.
 * All rights reserved.
 *
 * For the applicable distribution terms see the license text file included in
 * the distribution.
 */

#include "wlanclient.h"

#include <QTcpSocket>
#include <QTimer>
#include <QStringList>
#include "wlannetworkmgr.h"

#include "common.h"

//Constants
const int NumberOfRetries(3);
const int RetryInterval(500); //Milliseconds

/*!
  \class WlanClient
  \brief TODO
*/

/*!
  Constructor.
*/
WlanClient::WlanClient(QObject *parent) :
    QObject(parent),
    mSocket(0),
    mRetries(0),
    mClientStarted(false),
    mLastErrorString("")
{
}

/*!
  Destructor.
*/
WlanClient::~WlanClient()
{
    stopClient();
}

QString WlanClient::errorString() const
{
    return mLastErrorString;
}

/*!
  Initializes the client and connects to server given in \a serverInfo.
*/
void WlanClient::startClient(const NetworkServerInfo &serverInfo)
{
    if (mSocket || mRetries) {
        qDebug() << "WlanClient::startClient(): Already running!";
        return;
    }

    mServerInfo = serverInfo;
    mRetries = NumberOfRetries;
    mClientStarted = true;
    mLastErrorString = "";

    Common::resetBuffer();

    qDebug() << "WlanClient::startClient(): Network address:" << mServerInfo.address().toString()
             << "port:" << mServerInfo.port();


    mSocket = new QTcpSocket(this);
    connect(mSocket, SIGNAL(connected()), this, SLOT(onConnected()));
    connect(mSocket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
    connect(mSocket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(mSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onSocketError(QAbstractSocket::SocketError)));

    QTimer::singleShot(RetryInterval, this, SLOT(connectToServer()));
}

/*!
  Stops the client. The connection is terminated.
*/
void WlanClient::stopClient()
{
    if (mSocket) {
        qDebug() << "WlanClient::stopClient(): Disconnecting...";
        mSocket->disconnectFromHost();
        delete mSocket;
        mSocket = 0;
        mClientStarted = false;
    } else {
        qDebug() << "WlanClient::stopClient(): Not connected!";
    }
}

/*!
  Writes \a data to the open socket. Returns the number of bytes written or -1
  if failed to write any data.
*/
qint64 WlanClient::write(const QByteArray &data)
{
    return mSocket ? mSocket->write(data) : -1;
}

bool WlanClient::clientStarted() const
{
    return mClientStarted;
}

/*!
  Reads the data sent by the server.
*/
void WlanClient::onReadyRead()
{
    if (!mSocket) {
        qDebug() << "WlanClient::onReadyRead(): No socket!";
        return;
    }

    qDebug() << "WlanClient::onReadyRead(): =>";

    Common::readFromSocket(mSocket, this, "read", "WlanClient::onReadyRead():");

    qDebug() << "WlanClient::onReadyRead(): <=";
}

/*!
  This slot is called after a connection has been established succesfully.
*/
void WlanClient::onConnected()
{
    qDebug() << "WlanClient::onConnected(): Connected to"
             << mServerInfo.hostName() << "at"
             << (mServerInfo.address().toString() + ":" + QString::number(mServerInfo.port()));
    mRetries = 0;
    emit connectedToServer(mSocket->peerName());
}


/*!
  Disconnected from the server.
*/
void WlanClient::onDisconnected()
{
    qDebug() << "WlanClient::onDisconnected():" << mSocket->state();

    if (!mRetries) {
        emit disconnectedFromServer();
    }
}

/*!
  Trying to connect to a server.
*/
void WlanClient::connectToServer()
{
    qDebug() << "WlanClient::connectToServer(): Trying to connect to server"
             << mServerInfo.hostName() << "at"
             << (mServerInfo.address().toString() + ":" + QString::number(mServerInfo.port()));
    mSocket->connectToHost(mServerInfo.address(), mServerInfo.port());
}

/*!
  On error we'll try reconnecting while we have retries left.
*/
void WlanClient::onSocketError(QAbstractSocket::SocketError error)
{
    qDebug() << "WlanClient::onSocketError():" << error;

    if (mRetries > 0) {
        --mRetries;

        qDebug() << "WlanClient::onSocketError(): Will try" << mRetries
                 << "more time(s).";
        QTimer::singleShot(RetryInterval, this, SLOT(connectToServer()));
    } else {
        mLastErrorString = mSocket->errorString();
        emit socketError((int) error);
    }
}
