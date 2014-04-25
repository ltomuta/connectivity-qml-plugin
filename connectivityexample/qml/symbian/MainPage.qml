/**
 * Copyright (c) 2012-2014 Microsoft Mobile.
 */

import QtQuick 1.1
import com.nokia.symbian 1.1
import ConnectivityPlugin 1.0

import "qrc:/common"

Page {
    id: root
    ListHeading {
        id: pageHeading

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }

        z: 2

        ListItemText {
            anchors.fill: pageHeading.paddingItem
            role: "Heading"
            text: "Connectivity Plug-in Example"
        }
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
                margins: 10
            }

            // Connection type selector
            SelectionListItem {
                x: -10
                title: qsTr("Connection type");

                subTitle: {
                    var retval = "Undefined";

                    switch (connectionManager.connectionType) {
                    case ConnectionManager.Bluetooth:
                        retval = "Bluetooth";
                        break;
                    case ConnectionManager.LAN:
                        retval = "LAN";
                        break;
                    }

                    return retval;
                }

                onClicked: connTypeSelectionDialog.open();

                SelectionDialog {
                    id: connTypeSelectionDialog
                    titleText: qsTr("Select connection type");
                    selectedIndex: -1

                    model: ListModel {
                        ListElement { name: "Bluetooth"; }
                        ListElement { name: "LAN"; }
                    }

                    onSelectedIndexChanged: {
                        console.debug("connTypeSelectionDialog: onSelectedIndexChanged: " + connTypeSelectionDialog.selectedIndex);
                        connectionManager.connectionType = connTypeSelectionDialog.selectedIndex;

                        //Here we check if if there are too many ListElement in our ListModel depending on the type of connection.
                        //Lan connections get the extra Don't Care option.
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

            // Connect as (client/server) selector
            SelectionListItem {
                x: -10
                title: qsTr("Connect as");

                subTitle: {
                    var retval = "Undefined";

                    switch (connectionManager.connectAs) {
                    case ConnectionManager.Client:
                        retval = "Client";
                        break;
                    case ConnectionManager.Server:
                        retval = "Server";
                        break;
                    case ConnectionManager.DontCare:
                        retval = "Don't care";
                    }

                    return retval;
                }

                onClicked: {
                    if (connectionManager.isBluetooth &&
                        connAsModel.count > 2)
                    {
                        connAsModel.remove(connAsModel.count - 1);
                    }

                    if (connectionManager.isLAN &&
                        connAsModel.count === 3 &&
                        connAsModel.get(2).name === "")
                    {
                        connAsModel.get(2).name = "Don't care"
                    }

                    connAsSelectionDialog.open();
                }

                SelectionDialog {
                    id: connAsSelectionDialog
                    titleText: qsTr("Connect as");
                    selectedIndex: -1

                    //On some symbian devices SelectionDialog doesn't resize properly
                    //so we create enough space for all 3 options.
                    //This makes the selection dialog look a little silly with
                    //2 options on some devices but atleast 3rd option is visible.
                    model: ListModel {
                        id: connAsModel
                        ListElement { name: "Client"; }
                        ListElement { name: "Server"; }
                        ListElement { name: ""; }
                    }

                    onSelectedIndexChanged: {
                        console.debug("connAsSelectionDialog: onSelectedIndexChanged: " + connAsSelectionDialog.selectedIndex);
                        connectionManager.connectAs = connAsSelectionDialog.selectedIndex;
                    }
                }
            }

            // Spacer
            Item {
                width: parent.width
                height: 10
            }

            Button {
                x: (parent.width - width) / 2
                width: 250
                text: connectionManager.status == ConnectionManager.NotConnected ?
                          qsTr("Connect") : qsTr("Disconnect");

                onClicked: {
                    if (connectionManager.status == ConnectionManager.NotConnected) {
                        connectionManager.connect();
                    }
                    else {
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
        privateCloseIcon: true
        onClickedOutside: aboutDialog.close()
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
        ToolButton {
            iconSource: "toolbar-back"
            onClicked: Qt.quit();
        }
        ToolButton {
            iconSource: Qt.resolvedUrl("start_chat.svg")
            opacity: connectionManager.isConnected ? 1.0 : 0.5
            enabled: connectionManager.isConnected
            onClicked: {
                pageStack.push(Qt.resolvedUrl("ChatPage.qml"))
            }
        }
        ToolButton {
            iconSource: Qt.resolvedUrl("info.svg")
            onClicked: {
                aboutDialog.open()
            }
        }
    }
}
