/**
 * Copyright (c) 2012-2014 Microsoft Mobile.
 * All rights reserved.
 *
 * For the applicable distribution terms see the license text file included in
 * the distribution.
 */

#include "bluetoothclient.h"

#include <QDebug>
#include <QStringList>
#include <QTimer>

#include "common.h"

#if !defined(Q_WS_SIMULATOR) && !defined(DISABLE_BLUETOOTH)
#include <qbluetoothdeviceinfo.h>
#endif

// Constants
const int NumberOfRetries(3);
const int RetryInterval(500); // Milliseconds


/*!
  \class BluetoothClient
  \brief Implementation of a bluetooth client which can connect to services.
*/

/*!
  Constructor.
*/
BluetoothClient::BluetoothClient(QObject *parent)
    : QObject(parent),
      mSocket(0),
      mRetries(0),
      mLastErrorString("")
{
}

/*!
  Destructor.
*/
BluetoothClient::~BluetoothClient()
{
    stopClient();
}

QString BluetoothClient::errorString() const
{
    return mLastErrorString;
}

/*!
  Initializes the client and connects to \a remoteService.
*/
void BluetoothClient::startClient(const QBluetoothServiceInfo &remoteService)
{
    if (mSocket || mRetries) {
        qDebug() << "BluetoothClient::startClient(): Already running!";
        return;
    }

    mService = remoteService;
    mRetries = NumberOfRetries;
    mLastErrorString = "";

    Common::resetBuffer();

    QBluetoothAddress address = mService.device().address();
    qDebug() << "BluetoothClient::startClient(): Bluetooth address: " << address.toString();

    mSocket = new QBluetoothSocket(QBluetoothSocket::RfcommSocket);
    connect(mSocket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(mSocket, SIGNAL(connected()), this, SLOT(onConnected()));
    connect(mSocket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
    connect(mSocket, SIGNAL(error(QBluetoothSocket::SocketError)),
            this, SLOT(onSocketError(QBluetoothSocket::SocketError)));

    // mSocket->connectToService() call may block the UI thread. Thus, use the
    // the timer to delay the call in case we want to show some note for the
    // user.
    QTimer::singleShot(RetryInterval, this, SLOT(connectToService()));
}


/*!
  Stops the client. The connection is terminated.
*/
void BluetoothClient::stopClient()
{
    if (mSocket) {
        qDebug() << "BluetoothClient::stopClient(): Disconnecting...";
        mSocket->disconnectFromService();
        delete mSocket;
        mSocket = 0;
    }
    else {
        qDebug() << "BluetoothClient::stopClient(): Not connected!";
    }
}


/*!
  Writes \a data to the open socket. Returns the number of bytes written or -1
  if failed to write any data.
*/
qint64 BluetoothClient::write(const QByteArray &data)
{
    if (mSocket) {
        return mSocket->write(data);
    }

    return -1;
}


/*!
  Tries to connect to the set service. Returns the socket state after.
*/
int BluetoothClient::connectToService()
{
    qDebug() << "BluetoothClient::connectToService(): Trying to connect to service"
             << mService.device().name();
             //<< mSocket->peerName();
    mSocket->connectToService(mService);
    return mSocket->state();
}


/*!
  This slot is called after a connection has been established succesfully.
*/
void BluetoothClient::onConnected()
{
    qDebug() << "BluetoothClient::onConnected(): Connected to"
             << mSocket->peerName() << "; Socket state is" << mSocket->state();
    mRetries = 0;
    emit connectedToService(mSocket->peerName());
}


/*!
  Disconnected from the server.
*/
void BluetoothClient::onDisconnected()
{
    qDebug() << "BluetoothClient::onDisconnected():" << mSocket->state();

    if (!mRetries) {
        emit disconnectedFromServer();
    }
}


/*!
  Reads the data sent by the server.
*/
void BluetoothClient::onReadyRead()
{
    if (!mSocket) {
        qDebug() << "BluetoothClient::onReadyRead(): No socket!";
        return;
    }

    qDebug() << "BluetoothClient::onReadyRead(): =>";
    Common::readFromSocket(mSocket, this, "read",
                           "BluetoothClient::onReadyRead():");
    qDebug() << "BluetoothClient::onReadyRead(): <=";
}


/*!
  Error handler for socket errors.
*/
void BluetoothClient::onSocketError(QBluetoothSocket::SocketError error)
{
    qDebug() << "BluetoothClient::onSocketError():" << error;

    if (mRetries > 0) {
        mRetries--;
        qDebug() << "BluetoothClient::onSocketError(): Will try" << mRetries << "more time(s).";
        QTimer::singleShot(RetryInterval, this, SLOT(connectToService()));
    } else {
        mLastErrorString = mSocket->errorString();
        emit socketError((int) error);
    }
}

