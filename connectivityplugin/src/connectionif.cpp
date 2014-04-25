/**
 * Copyright (c) 2012-2014 Microsoft Mobile.
 * All rights reserved.
 *
 * For the applicable distribution terms see the license text file included in
 * the distribution.
 */

/*!
  \enum ConnectionIf::ConnectionStatus
  \value NotConnected   Not connected to any service.
  \value Discovering    Discovering services.
  \value Connecting     Establishing a connection to a service.
  \value Connected      Connected to a service.
  \value Disconnecting  Connection is in the process of shutting down.
*/

/*!
  \enum ConnectionIf::ConnectionType
  \value Bluetooth  Using Bluetooth connection.
  \value LAN        Using WLAN connection.
*/

/*!
  \enum ConnectionIf::ConnectAs
  \value Client     Connecting as a client to a service.
  \value Server     Starting a service for clients to connect to
  \value DontCare   Used only for LAN connection. Starting both client to search for existing
                    services and a service to wait for clients to connect.
*/

#include "connectionif.h"

#include <QDebug>

/*!
  \class ConnectionIf
  \brief An abstract interface for connection implementations.
*/

/*!
  Constructor.
*/
ConnectionIf::ConnectionIf(QObject *parent)
    : QObject(parent),
      mNetworkStatus(NetworkNotConnected),
      mStatus(NotConnected),
      mConnectAs(Client),
      mError(0),
      mMaxConnections(0)
{
}


/*!
  Sets the service information.
*/
void ConnectionIf::setServiceInfo(const QString &serviceName,
                                  const QString &serviceProvider)
{
    qDebug() << "ConnectionIf::setServiceInfo():"
             << serviceName << serviceProvider;
    mServiceName = serviceName;
    mServiceProvider = serviceProvider;
}


/*!
  Sets the status.
*/
void ConnectionIf::setStatus(ConnectionStatus status)
{
    mStatus = status;
    emit statusChanged(status);
}

void ConnectionIf::setNetworkStatus(ConnectionIf::NetworkStatus status)
{
    mNetworkStatus = status;
    emit networkStatusChanged(status);
}
