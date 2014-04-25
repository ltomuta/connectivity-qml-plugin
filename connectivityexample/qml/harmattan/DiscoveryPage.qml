/**
 * Copyright (c) 2012-2014 Microsoft Mobile.
 */

import QtQuick 1.1
import com.nokia.meego 1.1
import ConnectivityPlugin 1.0

Page {
    Header {
        id: header
        anchors {
            top: parent.top
            left: parent.left
        }
        width: parent.width
        text: "Discovered services"
        BusyIndicator {
            height: parent.height - 4
            width: height

            anchors {
                top: parent.top
                left: parent.left
                margins: 2
            }

            running: true
        }
    }

    ListView {
        id: listView
        anchors {
            top: header.bottom
            bottom: parent.bottom
            left: parent.left
            right: parent.right
            topMargin: 20
        }

        model: discoveredServicesModel
        spacing: 5
        delegate: Component {
            id: listItem
            Button {
                width: listView.width
                text: model.name
                onClicked: {
                    connectionManager.connect(name);
                    pageStack.pop();
                }
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
