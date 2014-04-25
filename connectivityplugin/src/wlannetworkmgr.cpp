/**
 * Copyright (c) 2012-2014 Microsoft Mobile.
 * All rights reserved.
 *
 * For the applicable distribution terms see the license text file included in
 * the distribution.
 */

#include "wlannetworkmgr.h"

#include <QDebug>

namespace
{
//Global network manager
static WlanNetworkMgr* instance = 0;
}

namespace Network
{
/*!
  A function that returns a global network manager.
  It is used as a workaround for a crash that happened when networkmanager was destroyed
  with WlanConnection while establishing a NetworkSession was still in progress.
*/
WlanNetworkMgr& networkManager()
{
    if (::instance == 0) {
        ::instance = new WlanNetworkMgr;
    }
    return *::instance;
}
/*!
 *Releases the global network manager
*/
void release()
{
    if (::instance != 0) {
        delete ::instance;
        ::instance = 0;
    }
}

}


/*!
  \class WlanNetworkMgr
  \brief Manages the connectivity.
*/


/*!
  Constructor.
*/
WlanNetworkMgr::WlanNetworkMgr(QObject *parent) :
    QObject(parent),
    mNetworkSession(0),
    mNetworkConfMgr(0),
    mAccessPoint(""),
    mIp(QHostAddress::Null),
    mState(QNetworkSession::Invalid)
{
    mIp.setAddress(QHostAddress::LocalHost);
    mNetworkConfMgr = new QNetworkConfigurationManager(this);

    QObject::connect(mNetworkConfMgr, SIGNAL(updateCompleted()),
                     this, SLOT(openNewNetworkSession()));
}


/*!
  Destructor.
*/
WlanNetworkMgr::~WlanNetworkMgr()
{
    disconnect();
}

/*!
  Disconnects (if connected) and updates the network configurations. Once the
  update is complete, the manager will try to automatically connect to a
  network.
*/
void WlanNetworkMgr::connectToNetwork()
{
    qDebug() << "WlanNetworkMgr::connectToNetwork(): =>";
    disconnect();
    mNetworkConfMgr->updateConfigurations();
    qDebug() << "WlanNetworkMgr::connectToNetwork(): <=";
}

/*!
  Returns true if \a configuration is a valid network configuration.
  False otherwise.
*/
bool WlanNetworkMgr::validConfiguration(QNetworkConfiguration &configuration) const
{
    bool valid =
#ifdef Q_OS_SYMBIAN
            (configuration.isValid() &&
             configuration.bearerType() == QNetworkConfiguration::BearerWLAN
             || configuration.type() == QNetworkConfiguration::UserChoice);
#elif defined(Q_WS_SIMULATOR) || defined(DISABLE_BLUETOOTH)
            (configuration.isValid());
#else
            (configuration.isValid() &&
             (configuration.bearerType() == QNetworkConfiguration::BearerWLAN
              || configuration.type() == QNetworkConfiguration::ServiceNetwork
              || configuration.type() == QNetworkConfiguration::UserChoice));
#endif

    if (valid) {
        qDebug() << "WlanNetworkMgr::validConfiguration():"
                 << configuration.name()
                 << configuration.bearerTypeName()
                 << configuration.bearerType()
                 << configuration.type()
                 << configuration.state()
                 << configuration.isValid();
    }

    return valid;
}

/*!
  Clears the connection information.
*/
void WlanNetworkMgr::clearConnectionInformation()
{
    qDebug() << "WlanNetworkMgr::clearConnectionInformation()";

    // Clear the access point and initialize own IP to localhost.
    mAccessPoint = QString();
    mIp.setAddress(QHostAddress::LocalHost);
    emit accessPointChanged(mAccessPoint);
    emit ipChanged(mIp.toString());
}


/*!
  Sets the state of the manager to \a state.
*/
void WlanNetworkMgr::setState(QNetworkSession::State state)
{
    if (state != mState) {
        mState = state;
        emit stateChanged(mState);
    }
}

/*!
  Opens a new network session.
*/
void WlanNetworkMgr::openNewNetworkSession()
{
    qDebug() << "WlanNetworkMgr::openNewNetworkSession() =>";

    if (mState == QNetworkSession::Connecting) {
        qDebug() << "WlanNetworkMgr::openNewNetworkSession()"
                 << "<= Already connecting!!!";
        return;
    }

    // Disconnect previous connection and delete the network session instance.
    disconnect();
    clearConnectionInformation();
    delete mNetworkSession;
    mNetworkSession = 0;

    // Set state to 'Connecting'.
    setState(QNetworkSession::Connecting);

    // Get the configurations.
    QList<QNetworkConfiguration> configurations =
            mNetworkConfMgr->allConfigurations(QNetworkConfiguration::Discovered);

    if (configurations.isEmpty()) {
        configurations << mNetworkConfMgr->defaultConfiguration();
    }

    bool sessionOpened(false);

    foreach (QNetworkConfiguration configuration, configurations) {
        if (!validConfiguration(configuration)) {
            // Invalid configuration, try the next one.
            continue;
        }

        qDebug() << "WlanNetworkMgr::openNewNetworkSession(): Opening network session.";

        // Valid configuration found!
        mNetworkSession = new QNetworkSession(configuration, this);

        // Open the network session.
        mNetworkSession->open();

        if (mNetworkSession->waitForOpened()) {
            qDebug() << "WlanNetworkMgr::openNewNetworkSession():"
                     << "Selecting" << configuration.name();

            // Connect the signals.
            connect(mNetworkSession, SIGNAL(closed()),
                    this, SLOT(handleNetworkSessionClosed()),
                    Qt::UniqueConnection);
            connect(mNetworkSession, SIGNAL(error(QNetworkSession::SessionError)),
                    this, SLOT(handleError(QNetworkSession::SessionError)),
                    Qt::UniqueConnection);
            connect(mNetworkSession, SIGNAL(newConfigurationActivated()),
                    this, SLOT(handleNewConfigurationActivated()),
                    Qt::UniqueConnection);
            connect(mNetworkSession, SIGNAL(stateChanged(QNetworkSession::State)),
                    this, SLOT(handleStateChanged(QNetworkSession::State)),
                    Qt::UniqueConnection);

            handleNetworkSessionOpened();
            sessionOpened = true;

            break;
        }
        else {
            qDebug() << "WlanNetworkMgr::openNewNetworkSession():"
                     << "Failed to open" << configuration.name()
                     << ":" << mNetworkSession->errorString()
                     << mNetworkSession->error();

            delete mNetworkSession;
            mNetworkSession = 0;
        }
    }

    if (!sessionOpened) {
        qDebug() << "WlanNetworkMgr::openNewNetworkSession():"
                 << "No valid session opened!";
        setState(QNetworkSession::Invalid);
    }

    qDebug() << "WlanNetworkMgr::openNewNetworkSession() <=";
}

/*!
  Disconnects if connected.
*/
void WlanNetworkMgr::disconnect()
{
    if (mNetworkSession && mNetworkSession->isOpen()) {
        qDebug() << "WlanNetworkMgr::disconnect(): Disconnecting...";
        mNetworkSession->close();
        setState(QNetworkSession::Closing);
    }
}

/*!
  Handles the changed \a state of the network session.
*/
void WlanNetworkMgr::handleStateChanged(QNetworkSession::State state)
{
    qDebug() << "WlanNetworkMgr::handleStateChanged():" << state;
    setState(state);
}


/*!
  Checks the configuration and updates the connection information if the
  connection is valid.
*/
void WlanNetworkMgr::handleNetworkSessionOpened()
{
    // Go through all interfaces and add all IPs for interfaces that can
    // broadcast and are not local.
    foreach (QNetworkInterface networkInterface,
             QNetworkInterface::allInterfaces())
    {
        if (!networkInterface.addressEntries().isEmpty()
            && networkInterface.isValid())
        {
            QNetworkInterface::InterfaceFlags interfaceFlags =
                    networkInterface.flags();

            if (interfaceFlags & QNetworkInterface::IsUp
                    && interfaceFlags & QNetworkInterface::IsRunning
                    && interfaceFlags & QNetworkInterface::CanBroadcast
                    && !(interfaceFlags & QNetworkInterface::IsLoopBack))
            {
                qDebug() << "WlanNetworkMgr::handleNetworkSessionOpened():"
                         << "The connection is valid!";

                if (mNetworkSession) {
                    mAccessPoint = mNetworkSession->configuration().name();
                    qDebug() << "WlanNetworkMgr::handleNetworkSessionOpened():"
                             << "Access point changed to" << mAccessPoint;
                    emit accessPointChanged(mAccessPoint);
                }

#if defined(Q_WS_SIMULATOR) || defined(DISABLE_BLUETOOTH)
                //We are looking for first valid IPv4 address
                foreach (QNetworkAddressEntry entry, networkInterface.addressEntries()) {                    
                    //if (entry.prefixLength() <= 32) {
                    if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                        mIp.setAddress(entry.ip().toString());
                        emit ipChanged(mIp.toString());
                        setState(QNetworkSession::Connected);
                        return;
                    }
                }
#else
                mIp.setAddress(networkInterface.addressEntries().at(0).ip().toString());
                emit ipChanged(mIp.toString());

                setState(QNetworkSession::Connected);
                return;                
#endif

            }
        }
    }

    qDebug() << "WlanNetworkMgr::handleNetworkSessionOpened():"
             << "The connection was not valid!";
    setState(QNetworkSession::Invalid);
}

/*!
  Updates the state to 'Disconnected' and deletes the network session instance.
*/
void WlanNetworkMgr::handleNetworkSessionClosed()
{
    qDebug() << "WlanNetworkMgr::handleNetworkSessionClosed(): =>";
    clearConnectionInformation();
    setState(QNetworkSession::Disconnected);
    qDebug() << "WlanNetworkMgr::handleNetworkSessionClosed(): <=";
}

/*!
  Updates the network configurations.
*/
void WlanNetworkMgr::handleNewConfigurationActivated()
{
    qDebug() << "WlanNetworkMgr::handleNewConfigurationActivated()";
    connectToNetwork();
}

/*!
  Handles \a error of the network session.
*/
void WlanNetworkMgr::handleError(QNetworkSession::SessionError error)
{
    qDebug() << "WlanNetworkMgr::handleError():" << error;

    if (mNetworkSession) {
        qDebug() << "WlanNetworkMgr::handleError():"
                 << mNetworkSession->errorString();
    }

    switch (error) {
        case QNetworkSession::UnknownSessionError: {
            break;
        }
        case QNetworkSession::SessionAbortedError: {
            break;
        }
        case QNetworkSession::RoamingError: {
            break;
        }
        case QNetworkSession::OperationNotSupportedError: {
            break;
        }
        case QNetworkSession::InvalidConfigurationError: {
            break;
        }
    }

    clearConnectionInformation();
    setState(QNetworkSession::Invalid);
}
