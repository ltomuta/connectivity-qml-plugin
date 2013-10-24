/**
 * Copyright (c) 2012 Nokia Corporation.
 * All rights reserved.
 *
 * For the applicable distribution terms see the license text file included in
 * the distribution.
 */

#include "connectivityplugin.h"
#include "connectionmanager.h"
#include <QDeclarativeEngine>
#include <QDeclarativeItem>

/*!
  \class ConnectivityPlugin
  \brief The main class of the plugin. Registers the public types.
*/

/*!
  Constructor.
*/
ConnectivityPlugin::ConnectivityPlugin(QObject *parent) :
    QDeclarativeExtensionPlugin(parent)
{
}

/*!
  From QDeclarativeExtensionPlugin. Registers the plugin under \a uri.
*/
void ConnectivityPlugin::registerTypes(const char *uri)
{
    // @uri ConnectivityPlugin 1.0
    qmlRegisterType<ConnectionManager>(uri, 1, 0, "ConnectionManager");
}

Q_EXPORT_PLUGIN2(ConnectionManager, ConnectivityPlugin)
