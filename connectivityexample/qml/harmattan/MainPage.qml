/**
 * Copyright (c) 2012 Nokia Corporation.
 */

import QtQuick 1.1
import com.nokia.meego 1.1
import ConnectivityPlugin 1.0
import "qrc:/common"

Page {
    id: root
    Header {
        id: pageHeading
        anchors {
            top: parent.top
            left: parent.left
        }
        width: parent.width
        text: "Connectivity Plug-in Example"
    }

    Rectangle {
        id: mainContainer
        height: 300

        anchors {
            top: pageHeading.bottom
            left: parent.left
            right: parent.right
        }

        z: 1
        color: "black"

        Column {
            id: columnContainer

            anchors {
                fill: parent
                leftMargin: 10
                rightMargin: 10
                bottomMargin: 10
                topMargin: 30
            }

            // Connection type selector
            Button {
                width: parent.width
                text: {
                    var retval = "Connection type: ";
                    switch (connectionManager.connectionType) {
                    case ConnectionManager.Bluetooth:
                        retval += "Bluetooth";
                        break;
                    case ConnectionManager.LAN:
                        retval += "LAN";
                        break;
                    }
                    return retval
                }
                onClicked: connTypeSelectionDialog.open();

                SelectionDialog {
                    id: connTypeSelectionDialog
                    titleText: qsTr("Select connection type");
                    selectedIndex: -1
                    platformStyle: SelectionDialogStyle {
                        itemHeight: 90
                    }

                    model: ListModel {
                        ListElement { name: "Bluetooth"; }
                        ListElement { name: "LAN"; }
                    }

                    onSelectedIndexChanged: {
                        console.debug("connTypeSelectionDialog: onSelectedIndexChanged: " + selectedIndex);
                        connectionManager.connectionType = selectedIndex;

                        if (connectionManager.isBluetooth && connAsModel.count > 2) {
                            if (connAsSelectionDialog.selectedIndex >= 2) {
                                --connAsSelectionDialog.selectedIndex;
                            }

                            connAsModel.remove(connAsModel.count - 1);
                        } else if (connectionManager.isLAN && connAsModel.count < 3) {
                            connAsModel.append({name: "Don't care"});
                        }
                    }
                }
            }

            // Spacer
            Item {
                width: parent.width
                height: 10
            }

            // Connect as (client/server) selector
            Button {
                width: parent.width
                text: {
                    var retval = "Connect as: ";

                    switch (connectionManager.connectAs) {
                    case ConnectionManager.Client:
                        retval += "Client";
                        break;
                    case ConnectionManager.Server:
                        retval += "Server";
                        break;
                    case ConnectionManager.DontCare:
                        retval += "Don't care";
                    }

                    return retval;
                }

                onClicked: connAsSelectionDialog.open();

                SelectionDialog {
                    id: connAsSelectionDialog
                    titleText: qsTr("Connect as");
                    selectedIndex: -1
                    platformStyle: SelectionDialogStyle {
                        itemHeight: 90
                    }

                    model: ListModel {
                        id: connAsModel
                        ListElement { name: "Client"; }
                        ListElement { name: "Server"; }
                    }

                    onSelectedIndexChanged: {
                        console.debug("connAsSelectionDialog: onSelectedIndexChanged: " + selectedIndex);
                        connectionManager.connectAs = selectedIndex;
                    }
                }
            }

            // Spacer
            Item {
                width: parent.width
                height: 20
            }

            Button {
                x: (parent.width - width) / 2
                width: 250
                text: connectionManager.status == ConnectionManager.NotConnected ?
                          qsTr("Connect") : qsTr("Disconnect");

                onClicked: {
                    if (connectionManager.status == ConnectionManager.NotConnected) {
                        connectionManager.connect();
                    } else {
                        connectionManager.disconnect();
                    }
                }
            }
        }
    }

    StatusInfo {
        anchors {
            top: mainContainer.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
    }

    CommonDialog {
        id: aboutDialog
        titleText: "About"
        content: Item {
            height: root.height / 2
            width: parent.width
            About {
                id: cabout
                fontSize: 25
                anchors.fill: parent
            }
            ScrollDecorator {
                flickableItem: cabout
            }
        }
    }

    tools: ToolBarLayout {
        ToolIcon {
            iconId: "toolbar-back"
            onClicked: Qt.quit();
        }

        ToolIcon {
            opacity: connectionManager.isConnected ? 1.0 : 0.5
            enabled: connectionManager.isConnected
            iconSource: Qt.resolvedUrl("start_chat.png")
            onClicked: {
                pageStack.push(Qt.resolvedUrl("ChatPage.qml"))
            }
        }

        ToolIcon {
            iconSource: Qt.resolvedUrl("info.png")
            onClicked: {
                aboutDialog.open()
            }
        }
    }
}
