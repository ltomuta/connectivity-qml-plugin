/**
 * Copyright (c) 2012-2014 Microsoft Mobile.
 */

import QtQuick 1.1
import com.nokia.symbian 1.1
import ConnectivityPlugin 1.0

Page {

    ListView {
        id: listView
        anchors.fill: parent
        model: discoveredServicesModel

        header: ListHeading {
            id: listHeading

            ListItemText {
                anchors.fill: listHeading.paddingItem
                role: "Heading"
                text: "Discovered services"
            }

            BusyIndicator {
                width: height
                height: parent.height - 4

                anchors {
                    top: parent.top
                    left: parent.left
                    margins: 2
                }

                running: true
            }
        }

        delegate: ListItem {
            id: listItem

            ListItemText {
                anchors.fill: listItem.paddingItem
                role: "Title"
                text: name
            }

            onClicked: {
                connectionManager.connect(name);
                pageStack.pop();
            }
        }

    }
    ScrollDecorator {
        flickableItem: listView
    }

    tools: ToolBarLayout {
        ToolButton {
            text: qsTr("Cancel");

            onClicked: {
                connectionManager.disconnect();
                pageStack.pop();
            }
        }
    }
}
