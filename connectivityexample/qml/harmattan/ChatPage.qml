/**
 * Copyright (c) 2012 Nokia Corporation.
 */

import QtQuick 1.1
import com.nokia.meego 1.0
import ConnectivityPlugin 1.0
import "qrc:/common"

Page {
    Header {
        id: header
        anchors {
            top: parent.top
            left: parent.left
        }
        width: parent.width
        z: 1
        text: {
            var name = connectionManager.peerName;

            if (name.length == 0) {
                name = "N/A";
            }

            return "Chatting with: " + name;
        }
    }

    ScrollDecorator {
        flickableItem: listView
    }

    MessageView {
        id: listView
        width: parent.width
        anchors.top: header.bottom
        anchors.bottom: writeMessage.top
        delegate: MessageBubble {
            maxWidth: 350
            messageFontSize: 22
            metaFontSize: 18
        }
        model: myMessageModel
    }

    Rectangle {
        z: 1
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: writeMessage.height + 10
        color: "black"
    }

    TextField {
        id: writeMessage
        z: 2
        font.pixelSize: 22
        anchors {
            bottom: parent.bottom
            left: parent.left
            right: parent.right
            margins: 10
        }
        text: ""
        placeholderText: "Type here to chat."

        platformStyle: TextFieldStyle {
            paddingRight: sendButton.width * 1.1
        }

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
        listView.focus = true

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
        ToolIcon {
            iconId: "toolbar-back"
            onClicked: pageStack.pop()
        }        
    }
}
