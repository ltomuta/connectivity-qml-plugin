/**
 * Copyright (c) 2012-2014 Microsoft Mobile.
 */

import QtQuick 1.1
//import com.nokia.symbian 1.1
import ConnectivityPlugin 1.0

Rectangle {
    anchors {
        left: parent.left
        right: parent.right
    }

    color: "#666666"

    // Border and shadow
    Rectangle {
        id: border
        width: parent.width
        height: 1
        anchors.top: parent.top
        z: 1
        color: "#444444"
    }
    Rectangle {
        width: parent.width
        height: 5
        anchors.top: border.bottom
        z: 1

        gradient: Gradient {
            GradientStop { position: 0; color: "#aa000000"; }
            GradientStop { position: 1; color: "#00000000"; }
        }
    }

    Flickable {
        id: flickable
        anchors.fill: parent
        contentHeight: 200

        Grid {
            anchors {
                fill: parent
                margins: 10
            }

            rows: 3
            columns: 2
            spacing: 10

            Text {
                id: connectionStatusLabel
                width: parent.width / 2 - 5
                color: "white"
                font.pixelSize: 20
                font.bold: true
                text: "Connection status:"
            }
            Text {
                id: connectionStatusText
                width: parent.width / 2 - 5
                color: "#dddddd"
                font.pixelSize: 20

                text: {
                    var retval;

                    switch (connectionManager.status) {
                    case ConnectionManager.NotConnected:
                        retval = "Not connected";
                        break;
                    case ConnectionManager.Discovering:
                        retval = "Discovering";
                        break;
                    case ConnectionManager.Connecting:
                        retval = "Connecting";
                        break;
                    case ConnectionManager.Connected:
                        myMessageModel.clear()
                        retval = "Connected";
                        break;
                    case ConnectionManager.Disconnecting:
                        retval = "Disconnecting";
                        break;
                    default:
                        retval = "Undefined";
                    }

                    return retval;
                }
            }
            Text {
                width: connectionStatusLabel.width
                color: connectionStatusLabel.color
                font.pixelSize: connectionStatusLabel.font.pixelSize
                font.bold: connectionStatusLabel.font.bold
                text: "Peer name:"
            }
            Text {
                width: connectionStatusText.width
                color: connectionStatusText.color
                font.pixelSize: connectionStatusText.font.pixelSize

                text: {
                    var name = connectionManager.peerName;

                    if (name.length == 0) {
                        name = "n/a";
                    }

                    return name;
                }
            }
            Text {
                width: connectionStatusLabel.width
                color: connectionStatusLabel.color
                font.pixelSize: connectionStatusLabel.font.pixelSize
                font.bold: connectionStatusLabel.font.bold
                text: "Latest error:"
            }
            Text {
                width: connectionStatusText.width
                color: connectionStatusText.color
                font.pixelSize: connectionStatusText.font.pixelSize
                text: connectionManager.error
            }
        }
    }
}
