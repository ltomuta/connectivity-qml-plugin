/**
 * Copyright (c) 2012 Nokia Corporation.
 * All rights reserved.
 *
 * For the applicable distribution terms see the license text file included in
 * the distribution.
 */

#ifndef CONNECTIONIF_H
#define CONNECTIONIF_H

#include <QObject>
#include <QString>
#include <QByteArray>


class ConnectionIf : public QObject
{
    Q_OBJECT

public:
    enum ConnectionStatus {
        NotConnected = 0,
        Discovering,
        Connecting,
        Connected,
        Disconnecting
    };

    enum ConnectionType {
        Bluetooth = 0,
        LAN
    };

    enum ConnectAs {
        Client = 0,
        Server,
        DontCare
    };

    enum NetworkStatus {
        NetworkNotConnected = 0,
        NetworkConnecting,
        NetworkConnected,
        NetworkDisconnecting
    };

public:
    explicit ConnectionIf(QObject *parent = 0);

public:
    NetworkStatus networkStatus() const { return mNetworkStatus; }
    ConnectionStatus status() const { return mStatus; }
    virtual void setConnectAs(ConnectAs connectAs) { mConnectAs = connectAs; }
    virtual void setServiceInfo(const QString &serviceName,
                                const QString &serviceProvider);

    virtual void setMaxConnections(int max) {mMaxConnections = max;}
    int maxConnections() const {return mMaxConnections;}

    QString connectedTo() const {return mConnectedTo;}
    QString localName() const {return mLocalName;}

    virtual int error() const { return mError; }
    virtual QString errorString() const { return mErrorString; }
    virtual ConnectionType type() const = 0;

public slots:
    virtual bool connect() = 0;
    virtual void disconnect() = 0;
    virtual bool send(const QByteArray &message) = 0;

protected slots:
    virtual void setStatus(ConnectionStatus status);
    virtual void setNetworkStatus(NetworkStatus status);

signals:
    void networkStatusChanged(NetworkStatus status);
    void statusChanged(ConnectionStatus status);
    void received(const QString &message);
    void errorOccured(int error);

protected: // Data
    NetworkStatus mNetworkStatus;
    ConnectionStatus mStatus;
    ConnectAs mConnectAs;
    QString mServiceName;
    QString mServiceProvider;
    QString mConnectedTo;
    QString mLocalName;
    QString mErrorString;
    int mError;
    int mMaxConnections;
};

#endif // CONNECTIONIF_H
