/**
 * Copyright (c) 2012 Nokia Corporation.
 * All rights reserved.
 *
 * For the applicable distribution terms see the license text file included in
 * the distribution.
 */

#ifndef WLANCONNECTION_H
#define WLANCONNECTION_H

#include "connectionif.h"
#include <QObject>
#include <QNetworkSession>


#include "networkserverinfo.h"

class WlanServer;
class WlanClient;
class WlanDiscoveryMgr;

class WlanConnection : public ConnectionIf
{
    Q_OBJECT

public:
    explicit WlanConnection(QObject *parent = 0);
    ~WlanConnection();

public:
    void setConnectAs(ConnectAs connectAs);
    ConnectionType type() const;
    int serverPort() const;
    int broadcastPort() const;
    void setMaxConnections(int max);

public slots:
    bool connect();
    bool connectToServer(const QString& info);
    void disconnect();
    bool send(const QByteArray &message);
    void setServerPort(int port);
    void setBroadcastPort(int port);

private slots:
    void onServerFound(NetworkServerInfo info);
    void onServerExists(NetworkServerInfo info);
    void onServerRemoved(int index);
    void onRead(const QByteArray &data);
    void onConnected(const QString &peer);
    void onClientConnected(const QString &peer);
    void onClientDisconnected(int remainingClients);
    void onDisconnected();
    void onReconnect();
    void onIpChanged(QString ip);
    void onSocketError(int error);

    void startServer();
    void startClient();
    void startDiscoveryMgr();

    void onNetworkStateChanged(QNetworkSession::State state);

signals:
    void discovered(const QString &hostName);
    void removed(int index);

private: // Data
    int mServerPort;
    int mBroadcastPort;

    WlanServer *mServer; //Owned
    WlanClient *mClient; //Owned
    WlanDiscoveryMgr *mDiscoveryMgr; //Owned
};

#endif // WLANCONNECTION_H
