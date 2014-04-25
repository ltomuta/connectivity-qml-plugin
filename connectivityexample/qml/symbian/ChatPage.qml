/**
 * Copyright (c) 2012-2014 Microsoft Mobile.
 */

import QtQuick 1.1
import com.nokia.symbian 1.1
import ConnectivityPlugin 1.0
import "qrc:/common"

Page {
    // Header that shows name of the chat partner
    ListHeading {
        id: listHeading
        anchors.top: parent.top
        z: 1
        ListItemText {
            id: headingText
            anchors.fill: listHeading.paddingItem
            role: "Heading"
            text: {
                var name = connectionManager.peerName;

                if (name.length == 0) {
                    name = "N/A";
                }

                return "Chatting with: " + name;
            }
        }
    }

    Item {
        id: dummy
    }

    // ScrollBar
    ScrollDecorator {
         id: scrolldecorator
         flickableItem: listView
     }

    //Messageview
    MessageView {
        id: listView
        width: parent.width
        anchors.top: listHeading.bottom
        anchors.bottom: writeMessage.top
        delegate: MessageBubble {
            maxWidth: 250
            messageFontSize: platformStyle.fontSizeMedium
            metaFontSize: platformStyle.fontSizeSmall
        }
        model: myMessageModel

        //Mouse area to close the inputfield when listview gets clicked
        MouseArea {
            anchors.fill: parent
            onClicked: {
                dummy.focus = true
                writeMessage.closeSoftwareInputPanel();
            }
        }
    }

    Rectangle {
        z: 1
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: writeMessage.height + 10
        color: "black"
    }


    // TextField to write new messages
    TextField {
        id: writeMessage
        z: 2
        font.pixelSize: platformStyle.fontSizeMedium
        anchors {
            bottom: parent.bottom;
            left: parent.left;
            right: parent.right
            margins: 10
        }

        text: ""
        placeholderText: "Type here to chat."
        platformRightMargin: sendButton.width
        SendButton {
            id: sendButton
            anchors.top: parent.top
            anchors.right: parent.right
            onClicked: {
                sendMessage();
            }
        }
    }

    function sendMessage() {
        listView.forceActiveFocus()

        if(writeMessage.text != "")
        {
            if (connectionManager.send(writeMessage.text)) {
                myMessageModel.append({"message": writeMessage.text, "me": true,
                                       "metaString": Qt.formatDateTime(new Date(), "hh:mm | dd.MM.yyyy")})
            }
        }

        writeMessage.text = ""
        listView.positionViewAtEnd()
    }

    tools: ToolBarLayout {
        ToolButton {
            iconSource: "toolbar-back"
            onClicked: pageStack.pop()
        }
    }
}
