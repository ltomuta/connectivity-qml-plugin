/**
 * Copyright (c) 2012 Nokia Corporation.
 * All rights reserved.
 *
 * For the applicable distribution terms see the license text file included in
 * the distribution.
 */

#ifndef WLANCLIENT_H
#define WLANCLIENT_H

#include <QObject>
#include <QAbstractSocket>

#include "networkserverinfo.h"

class QTcpSocket;

class WlanClient : public QObject
{
    Q_OBJECT
public:
    explicit WlanClient(QObject *parent = 0);    
    ~WlanClient();

    QString errorString() const;
    
public slots:
    void startClient(const NetworkServerInfo &serverInfo);
    void stopClient();
    qint64 write(const QByteArray &data);
    bool clientStarted() const;

private slots:
    void onReadyRead();
    void onConnected();
    void onDisconnected();
    void connectToServer();
    void onSocketError(QAbstractSocket::SocketError error);

signals:
    void read(const QByteArray &data);
    void connectedToServer(const QString &name);
    void disconnectedFromServer();
    void socketError(int error);

private: //Data
    QTcpSocket *mSocket; //Owned
    NetworkServerInfo mServerInfo;
    int mRetries;
    bool mClientStarted;
    QString mLastErrorString;
};

#endif // WLANCLIENT_H
