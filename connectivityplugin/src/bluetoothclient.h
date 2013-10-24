/**
 * Copyright (c) 2012 Nokia Corporation.
 * All rights reserved.
 *
 * For the applicable distribution terms see the license text file included in
 * the distribution.
 */

#ifndef BLUETOOTHCLIENT_H
#define BLUETOOTHCLIENT_H

#include <QByteArray>
#include <QObject>
#include <QVariant>

#if defined(Q_WS_SIMULATOR) || defined(DISABLE_BLUETOOTH)
#include "bluetoothstubs.h"
#else
#include <qbluetoothserviceinfo.h>
#include <qbluetoothsocket.h>

QTM_BEGIN_NAMESPACE
class QBluetoothSocket;
QTM_END_NAMESPACE

QTM_USE_NAMESPACE
#endif


class BluetoothClient : public QObject
{
    Q_OBJECT

public:
    explicit BluetoothClient(QObject *parent = 0);
    ~BluetoothClient();    

    QString errorString() const;
public slots:
    void startClient(const QBluetoothServiceInfo &remoteService);
    void stopClient();
    qint64 write(const QByteArray &data);

private slots:
    int connectToService();
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onSocketError(QBluetoothSocket::SocketError error);

signals:
    void connectedToService(const QString &name);
    void disconnectedFromServer();
    void read(const QByteArray &data);
    void socketError(int error);

private:
    QBluetoothSocket *mSocket; // Owned
    QBluetoothServiceInfo mService;
    int mRetries;
    QString mLastErrorString;
};


#endif // BLUETOOTHCLIENT_H
