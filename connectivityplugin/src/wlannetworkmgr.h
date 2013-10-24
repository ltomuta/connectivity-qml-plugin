/**
 * Copyright (c) 2012 Nokia Corporation.
 * All rights reserved.
 *
 * For the applicable distribution terms see the license text file included in
 * the distribution.
 */

#ifndef WLANNETWORKMGR_H
#define WLANNETWORKMGR_H

#include <QNetworkConfiguration>
#include <QNetworkConfigurationManager>
#include <QNetworkSession>
#include <QObject>
#include <QString>

class WlanNetworkMgr : public QObject
{
    Q_OBJECT
public:
    explicit WlanNetworkMgr(QObject *parent = 0);
    ~WlanNetworkMgr();

    QNetworkSession::State state() const {return mState;}
    QString accessPoint() const {return mAccessPoint;}
    QString ip() const {return mIp.toString();}
    
public slots:
    void connectToNetwork();
    void disconnect();

private:
    bool validConfiguration(QNetworkConfiguration &configuration) const;
    void clearConnectionInformation();
    void setState(QNetworkSession::State state);

private slots:
    void openNewNetworkSession();
    void handleStateChanged(QNetworkSession::State state);
    void handleNetworkSessionOpened();
    void handleNetworkSessionClosed();
    void handleNewConfigurationActivated();
    void handleError(QNetworkSession::SessionError error);

signals:
    void stateChanged(QNetworkSession::State state);
    void accessPointChanged(QString accessPoint);
    void ipChanged(QString ip);

private: //Data
    QNetworkSession *mNetworkSession; //Owned
    QNetworkConfigurationManager *mNetworkConfMgr;
    QString mAccessPoint;
    QHostAddress mIp;
    QNetworkSession::State mState;
};

namespace Network
{
WlanNetworkMgr& networkManager();
void release();
}

#endif // WLANNETWORKMGR_H
