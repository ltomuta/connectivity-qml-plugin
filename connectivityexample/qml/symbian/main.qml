/**
 * Copyright (c) 2012-2014 Microsoft Mobile.
 */

import QtQuick 1.1
import com.nokia.symbian 1.1
import ConnectivityPlugin 1.0

PageStackWindow {
    id: root
    showStatusBar: true
    showToolBar: true
    platformSoftwareInputPanelEnabled: true
    initialPage: Qt.resolvedUrl("MainPage.qml");

    ListModel {
        id: discoveredServicesModel
    }

    // For storing chatmessages
    ListModel {
        id: myMessageModel
    }

    ConnectionManager {
        id: connectionManager
        maxConnections: 1
        property int prevStatus: ConnectionManager.NotConnected;

        property bool isBluetooth: (connectionManager.connectionType == ConnectionManager.Bluetooth);
        property bool isLAN : (connectionManager.connectionType == ConnectionManager.LAN);
        property bool isConnected: connectionManager.status == ConnectionManager.Connected;

        onErrorChanged: {
            console.debug("ConnectionManager::onErrorChanged " + errorString)
        }

        onNetworkStatusChanged: {
            console.debug("ConnectionManager::networkStatus changed to " + networkStatus)
        }

        onStatusChanged: {
            console.debug("ConnectionManager::status changed to " + status);
            console.debug("ConnectionManager::prevStatus was " + connectionManager.prevStatus);

            if (status == ConnectionManager.Discovering) {

                var connectionTypeOk = connectionManager.isBluetooth || connectionManager.isLAN;

                var connectAsOk =
                        (connectionManager.connectAs == ConnectionManager.Client) ||
                        (connectionManager.connectAs == ConnectionManager.DontCare);

                if (connectionTypeOk && connectAsOk)
                {
                    pageStack.push(Qt.resolvedUrl("DiscoveryPage.qml"));
                }
            }

            //If connectAs is DontCare and someone connects to us while
            //we are at discovery screen, we'll pop it back.
            if (status == ConnectionManager.Connected &&
                connectionManager.prevStatus == ConnectionManager.Discovering &&
                connectionManager.connectAs == ConnectionManager.DontCare)
            {
                pageStack.pop();
            }

            connectionManager.prevStatus = status;
        }

        onConnectionTypeChanged: {
            //When connection type changed we'll clear the servicesModel
            discoveredServicesModel.clear();
        }

        onDiscovered: {
            discoveredServicesModel.append({name : name});
        }

        onRemoved: {
            discoveredServicesModel.remove(index);
        }

        onReceived: {
            myMessageModel.append({"message": message, "me": false,
                                   "metaString": Qt.formatDateTime(new Date(),
                                                                   "hh:mm | dd.MM.yyyy")})
        }
    }
}
