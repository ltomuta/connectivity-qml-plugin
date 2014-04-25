/**
 * Copyright (c) 2012-2014 Microsoft Mobile.
 * All rights reserved.
 *
 * For the applicable distribution terms see the license text file included in
 * the distribution.
 */

#include "networkserverinfo.h"

#include <QStringList>
#include <QHostInfo>

/*!
  \class NetworkServerInfo
  \brief A simple helper class to contain server information.
*/

/*!
  Default constructor.
*/
NetworkServerInfo::NetworkServerInfo() :
    mHostName(""),
    mAddress(QHostAddress::Null),
    mPort(-1)
{
}


/*!
  Constructor taking the name of the \a host, \a address of the server and \a port.
*/
NetworkServerInfo::NetworkServerInfo(QString host,
                                     QHostAddress address, int port) :
    mHostName(host),
    mAddress(address),
    mPort(port)
{
}

/*!
  Constructor taking a \c QByteArray \a array separated by \a separator.
*/
NetworkServerInfo::NetworkServerInfo(const QByteArray &array, char separator)
{
    QString string(array);
    mHostName = readHostname(string, separator);
    mAddress = readAddress(string, separator);
    mPort = readPort(string, separator);
}

/*!
  Constructor taking a \c QString \a string separated by \a separator.
*/
NetworkServerInfo::NetworkServerInfo(const QString &string, char separator)
{
    mHostName = readHostname(string, separator);
    mAddress = readAddress(string, separator);
    mPort = readPort(string, separator);
}

/*!
  Reads hostname contained in \a info. \a info contains server data separated by \a separator.
  Returns the name of the server if found, otherwise an empty \c QString.
*/
QString NetworkServerInfo::readHostname(const QString &info, char separator)
{
    QStringList list = info.split(separator, QString::SkipEmptyParts);

    foreach (QString item, list) {
        bool ok = false;
        int port = item.toInt(&ok);
        QHostAddress a(QHostAddress::Null);

        if (item.toLower() != "localhost" &&
            !ok && port != -1 &&
            !a.setAddress(item))
        {
            return item;
        }
        //Order here is important because lookupHost is a blocking function
        //So we'll try to use it only if necessary
        QHostAddress hostInfo = lookupHost(item);
        if (!hostInfo.isNull()) {
            return item;
        }
    }

    return "";
}

/*!
  Reads address contained in \a info. \a info contains server data separated by \a separator.
  Returns the address if found, otherwise returns \c QHostAddress::Null;
*/
QHostAddress NetworkServerInfo::readAddress(const QString &info, char separator)
{
    QStringList list = info.split(separator, QString::SkipEmptyParts);

    foreach(QString item, list) {
        QHostAddress a(QHostAddress::Null);

        if (item.toLower() == "localhost") {
            return QHostAddress(QHostAddress::LocalHost);
        }

        if (a.setAddress(item)) {
            return a;
        }


        //Order here is important because lookupHost is a blocking function
        //So we'll try to use it only if necessary
        QHostAddress hostInfo = lookupHost(item);
        if (!hostInfo.isNull()) {
            return hostInfo;
        }
    }

    return QHostAddress(QHostAddress::Null);
}

/*!
  Reads port contained in \a info. \a info contains server data separated by \a separator.
  Returns the port if found, otherwise returns -1;
*/
int NetworkServerInfo::readPort(const QString &info, char separator)
{
    QStringList list = info.split(separator, QString::SkipEmptyParts);

    foreach (QString item, list) {
        bool ok = false;
        int port = item.toInt(&ok);
        if (ok) {
            return port;
        }
    }

    return -1;
}

/*!
  Looks up \a host using \a QHostInfo.
  Returns the first IPv4 address if found, \a QHostAddress::Null otherwise.
*/
QHostAddress NetworkServerInfo::lookupHost(const QString &host)
{
    QHostInfo info;
    QString hostLower = host.toLower();
    if (hostLower == "localhost") {
        QString local = QHostInfo::localHostName();
        info = QHostInfo::fromName(local);
    } else {
        info = QHostInfo::fromName(hostLower);
    }

    if (!info.addresses().empty()) {
        //Returns the first IPv4 address
        foreach (QHostAddress address, info.addresses()) {
            if (address.protocol() == QAbstractSocket::IPv4Protocol) {
                return address;
            }
        }
    }

    return QHostAddress(QHostAddress::Null);
}

/*!
  Returns the server information in \c QString separated by \a separator.
*/
QString NetworkServerInfo::toString(char separator) const
{
    return mHostName + separator + mAddress.toString() + separator + QString::number(mPort);
}

/*!
  Returns the hostname.
*/
QString NetworkServerInfo::hostName() const
{
    return mHostName;
}

/*!
  Returns the address.
*/
QHostAddress NetworkServerInfo::address() const
{
    return mAddress;
}

/*!
  Returns the port.
*/
int NetworkServerInfo::port() const
{
    return mPort;
}

/*!
  Sets the hostname to \a name.
*/
void NetworkServerInfo::setHostname(const QString &name)
{
    mHostName = name;
}

/*!
  Sets the address to \a address.
*/
void NetworkServerInfo::setAddress(const QHostAddress &address)
{
    mAddress = address;
}

/*!
  Sets the port to \a port.
*/
void NetworkServerInfo::setPort(int port)
{
    mPort = port;
}


/*!
  Returns true for a valid server information.
*/
bool NetworkServerInfo::isValid() const
{
    return (mPort != -1 && mAddress != QHostAddress::Null);
}
