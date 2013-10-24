/**
 * Copyright (c) 2012 Nokia Corporation.
 * All rights reserved.
 *
 * For the applicable distribution terms see the license text file included in
 * the distribution.
 */

#ifndef CONNECTIONMANAGER_H
#define CONNECTIONMANAGER_H

#include <QObject>
#include <QString>
#include <QTimer>

#include "connectionif.h"

class ConnectionManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int status READ status NOTIFY statusChanged)
    Q_PROPERTY(int networkStatus READ networkStatus NOTIFY networkStatusChanged)
    Q_PROPERTY(int connectionType READ connectionType WRITE setConnectionType NOTIFY connectionTypeChanged)
    Q_PROPERTY(int connectAs READ connectAs WRITE setConnectAs NOTIFY connectAsChanged)
    Q_PROPERTY(int connectionTimeout READ connectionTimeout WRITE setConnectionTimeout NOTIFY connectionTimeoutChanged)
    Q_PROPERTY(QString serviceName READ serviceName WRITE setServiceName NOTIFY serviceNameChanged)
    Q_PROPERTY(QString serviceProvider READ serviceProvider WRITE setServiceProvider NOTIFY serviceProviderChanged)
    Q_PROPERTY(QString peerName READ peerName NOTIFY peerNameChanged)
    Q_PROPERTY(int error READ error NOTIFY errorChanged)
    Q_PROPERTY(QString errorString READ errorString)
    Q_PROPERTY(QString localName READ localName)
    Q_PROPERTY(int serverPort READ serverPort WRITE setServerPort NOTIFY serverPortChanged)
    Q_PROPERTY(int broadcastPort READ broadcastPort WRITE setBroadcastPort NOTIFY broadcastPortChanged)
    Q_PROPERTY(int maxConnections READ maxConnections WRITE setMaxConnections NOTIFY maxConnectionsChanged)

    Q_ENUMS(ConnectionStatus)
    Q_ENUMS(ConnectionType)
    Q_ENUMS(ConnectAs)
    Q_ENUMS(NetworkStatus)

public: // Data types

    enum ConnectionStatus {
        NotConnected = ConnectionIf::NotConnected,
        Discovering = ConnectionIf::Discovering,
        Connecting = ConnectionIf::Connecting,
        Connected = ConnectionIf::Connected,
        Disconnecting = ConnectionIf::Disconnecting
    };

    enum NetworkStatus {
        NetworkNotConnected = ConnectionIf::NetworkNotConnected,
        NetworkConnecting = ConnectionIf::NetworkConnecting,
        NetworkConnected = ConnectionIf::NetworkConnected,
        NetworkDisconnecting = ConnectionIf::NetworkDisconnecting
    };

    enum ConnectionType {
        Bluetooth = ConnectionIf::Bluetooth,
        LAN = ConnectionIf::LAN
    };

    enum ConnectAs {
        Client = ConnectionIf::Client,
        Server = ConnectionIf::Server,
        DontCare = ConnectionIf::DontCare
    };

public:
    ConnectionManager(QObject *parent = 0);
    ~ConnectionManager();

public:
    int status() const;
    int networkStatus() const;
    int connectionType() const;
    void setConnectionType(int type);
    int connectAs() const;
    void setConnectAs(int connectAs);
    int connectionTimeout() const;
    void setConnectionTimeout(int timeout);
    QString serviceName() const;
    void setServiceName(const QString &name);
    QString serviceProvider() const;
    void setServiceProvider(const QString &provider);
    QString peerName() const;
    int error() const;
    QString errorString() const;
    QString localName() const;

    int serverPort() const;
    int broadcastPort() const;
    int maxConnections() const;

    void setServerPort(int port);
    void setBroadcastPort(int port);
    void setMaxConnections(int max);

public slots:
    void connect(const QString &to = QString());
    void disconnect(const QString &message = QString());
    bool send(const QString &message, bool header = true, bool compression = false);

private:
    void applySettings();

private slots:
    void setStatus(ConnectionStatus status);
    void onConnectionIfStatusChanged(ConnectionStatus status);
    void setNetworkStatus(NetworkStatus status);

signals:
    // Property signals
    void statusChanged(int status);
    void networkStatusChanged(int status);
    void connectionTypeChanged(int type);
    void connectAsChanged(int connectAs);
    void connectionTimeoutChanged(int timeout);
    void serviceNameChanged(const QString &name);
    void serviceProviderChanged(const QString &provider);
    void peerNameChanged(const QString &name);
    void errorChanged(int error);
    void serverPortChanged(int port);
    void broadcastPortChanged(int port);
    void maxConnectionsChanged(int max);

    // Other signals
    void disconnected();
    void received(const QString &message);
    void discovered(const QString &name);
    void removed(int index);

private: // Data
    ConnectionIf *mConnection; // Owned
    QTimer mTimeoutTimer;
    QString mServiceName;
    QString mServiceProvider;
    QString mPeerName;   
    ConnectionStatus mStatus;
    NetworkStatus mNetworkStatus;
    ConnectionType mConnectionType;
    ConnectionIf::ConnectAs mConnectAs;
    int mConnectionTimeout; // In seconds
    int mMaxConnections; //Max connections, 0 means accepting all.
    int mServerPort;
    int mBroadcastPort;
};

#endif // CONNECTIONMANAGER_H
