/**
 * Copyright (c) 2012 Nokia Corporation.
 * All rights reserved.
 *
 * For the applicable distribution terms see the license text file included in
 * the distribution.
 */

#ifndef NETWORKSERVERINFO_H
#define NETWORKSERVERINFO_H

#include <QObject>
#include <QHostAddress>

class NetworkServerInfo
{
public:
    NetworkServerInfo();
    NetworkServerInfo(QString host, QHostAddress address, int port);
    NetworkServerInfo(const QByteArray &array, char separator = ':');
    NetworkServerInfo(const QString &string, char separator = ':');

    QString toString(char separator = ':') const;
    QString hostName() const;
    QHostAddress address() const;
    int port() const;

    void setHostname(const QString &name);
    void setAddress(const QHostAddress &address);
    void setPort(int port);

    bool isValid() const;

private:    
    QString readHostname(const QString &info, char separator = ':');
    QHostAddress readAddress(const QString &info, char separator = ':');
    int readPort(const QString &info, char separator = ':');

    QHostAddress lookupHost(const QString &host);

    QString mHostName;
    QHostAddress mAddress;
    int mPort;    
};

inline bool operator==(const NetworkServerInfo &left, const NetworkServerInfo &right)
{
    return (left.hostName() == right.hostName()) &&
           (left.address() == right.address()) &&
           (left.port() == right.port());
}

#endif // NETWORKSERVERINFO_H
