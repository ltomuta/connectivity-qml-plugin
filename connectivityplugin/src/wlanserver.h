/**
 * Copyright (c) 2012 Nokia Corporation.
 * All rights reserved.
 *
 * For the applicable distribution terms see the license text file included in
 * the distribution.
 */

#ifndef WLANSERVER_H
#define WLANSERVER_H

#include <QObject>
#include <QList>
#include <QHostAddress>
#include <QDebug>
#include <QTimer>
#include <QNetworkSession>

#include "networkserverinfo.h"

//Forward declarations
class QTcpServer;
class QTcpSocket;
class QUdpSocket;

class WlanServer : public QObject
{
    Q_OBJECT
public:
    explicit WlanServer(QObject *parent = 0);
    ~WlanServer();

    QString clientName(int index) const;
    QString errorString() const;

public slots:
    bool startServer(int port, int bdport);
    void stopServer();
    qint64 write(const QByteArray &data);
    void onIpChanged(QString ip);
    void onServerNameChanged(QString serverName);
    void onNetworkStateChanged(QNetworkSession::State state);
    void onSocketError(QAbstractSocket::SocketError error);
    void onServerPortChanged(int port);
    void onBroadcastPortChanged(int port);
    void setMaxConnections(int max);
    void setState(QNetworkSession::State state);
    void setIp(const QString &ip);

private slots:
    void onNewConnection();
    void onDisconnected();
    void onReadyRead();
    void broadcastServerInfo();
    bool hasPeerAddress(const QHostAddress &address);

    void onNewDiscoveryConnection();

signals:
    void read(const QByteArray &data);
    void clientDisconnected(int remainingClients);
    void clientConnected(const QString &peerName);
    void reconnectToNetwork();
    void serverStopped();
    void socketError(int error);

private: //Data
    QTcpServer *mDiscoveryServer; //Owned
    QTcpServer *mTcpServer; //Owned
    QUdpSocket *mBroadcastSocket; //Owned
    QList<QTcpSocket*> mSockets; //Owned
    QTimer mBroadcastTimer;
    int mBroadcastPort;
    QNetworkSession::State mState;
    NetworkServerInfo mServerInfo;
    int mMaxConnections;
    QString mLastErrorString;
};

#endif // WLANSERVER_H
