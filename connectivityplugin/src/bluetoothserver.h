/**
 * Copyright (c) 2012 Nokia Corporation.
 * All rights reserved.
 *
 * For the applicable distribution terms see the license text file included in
 * the distribution.
 */

#ifndef BLUETOOTHSERVER_H
#define BLUETOOTHSERVER_H

#include <QObject>
#include <QtCore/QList>
#include <QByteArray>


#if defined(Q_WS_SIMULATOR) || defined(DISABLE_BLUETOOTH)
#include "bluetoothstubs.h"
#else
#include <qbluetoothserviceinfo.h>
#include <qbluetoothsocket.h>

QTM_BEGIN_NAMESPACE
class QRfcommServer;
QTM_END_NAMESPACE

QTM_USE_NAMESPACE
#endif


class BluetoothServer : public QObject
{
    Q_OBJECT

public:
    explicit BluetoothServer(QObject *parent = 0);
    ~BluetoothServer();

    QString clientName(int index) const;
    QString errorString() const;

public slots:
    void setServiceInfo(const QString &serviceName,
                        const QString &serviceProvider,
                        const QString &serviceDescription = QString("No description"));
    bool startServer();
    void stopServer();
    qint64 write(const QByteArray &data);
    void setMaxConnections(int max);

private slots:
    void onNewConnection();
    void onDisconnected();
    void onReadyRead();
    bool hasPeerName(const QString &name);
    void onSocketError(QBluetoothSocket::SocketError error);

signals:
    void clientConnected(const QString &name);
    void clientDisconnected(int remainingClients);
    void read(const QByteArray &data);
    void socketError(int error);

private: // Data
    QRfcommServer *mRfcommServer; // Owned
    QList<QBluetoothSocket*> mSockets; //Owned
    QBluetoothServiceInfo mServiceInfo;
    quint32 mServiceUuid;
    int mMaxConnections;
    QString mLastErrorString;
};

#endif // BLUETOOTHSERVER_H
