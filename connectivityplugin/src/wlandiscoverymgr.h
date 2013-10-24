/**
 * Copyright (c) 2012 Nokia Corporation.
 * All rights reserved.
 *
 * For the applicable distribution terms see the license text file included in
 * the distribution.
 */

#ifndef WLANDISCOVERYMGR_H
#define WLANDISCOVERYMGR_H

#include <QObject>
#include <QHostAddress>
#include <QList>
#include <QTimer>

#include "networkserverinfo.h"

class QUdpSocket;

class WlanDiscoveryMgr : public QObject
{
    Q_OBJECT
public:
    explicit WlanDiscoveryMgr(QObject *parent = 0);
    ~WlanDiscoveryMgr();

    Q_INVOKABLE NetworkServerInfo server(const NetworkServerInfo &info) const;
    Q_INVOKABLE NetworkServerInfo server(const QString &hostName) const;
    Q_INVOKABLE NetworkServerInfo server(const QHostAddress &address) const;
    Q_INVOKABLE NetworkServerInfo server(const int &port) const;

public slots:
    bool startDiscovery(int port);
    void stopDiscovery();

private slots:
    void readBroadcast();
    void checkServers();

signals:
    void serverFound(NetworkServerInfo info);
    void serverExists(NetworkServerInfo info);
    void serverRemoved(int index);
    
private: //Data
    QTimer mServerCheckTimer;
    QUdpSocket *mDiscoverySocket; //Owned
    QByteArray mBroadcastTitle;
    int mBroadcastPort;
    QList<NetworkServerInfo> mDiscoveredServers;
};

#endif // WLANDISCOVERYMGR_H
