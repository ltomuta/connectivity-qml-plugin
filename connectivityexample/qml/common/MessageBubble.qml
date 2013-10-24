/**
 * Copyright (c) 2012 Nokia Corporation.
 */

import QtQuick 1.1

Item {
    id: root

    // Check if the message is longer than time information and set the size according to that
    width: (messageField.width >= metaStringField.paintedWidth ? messageField.width + 20 : metaStringField.paintedWidth + 20)
    height: messageField.height + metaStringField.paintedHeight + 15

    // Anchor messages to left or right depending if it's received or sent message
    anchors.left: (!me ? parent.right : parent.left)
    anchors.leftMargin: (!me ? -bubbleImage.width - 10 : 10)

    //Some property defaults
    property int maxWidth: 350
    property int messageFontSize: 22
    property int metaFontSize: 18

    // Messages one by one
    Rectangle {
        id: messageBubble
        height: parent.height
        width: parent.width
        color: "transparent"

        // Message image
        BorderImage {
            id: bubbleImage
            source: (me ? "outgoing.png" : "incoming.png")
            width: parent.width + 10
            height: parent.height
            border {left: 30; top: 30; right: 30; bottom: 30;}
            horizontalTileMode: BorderImage.Stretch
            verticalTileMode: BorderImage.Stretch
        }

        // Actual message
        Text {
            id: messageField
            font.pixelSize: root.messageFontSize
            height: paintedHeight
            width: 500
            anchors.left: (!me ? parent.right : parent.left)
            anchors.leftMargin: (!me ? -paintedWidth : 20)
            anchors.top: parent.top
            anchors.topMargin: (!me ? 10 : 5)
            text: message
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere

            // See if the text is too long and needs to be cut and set the field width right
            Component.onCompleted: {
                if (messageField.paintedWidth > root.maxWidth) {
                    messageField.width = root.maxWidth
                } else {
                    messageField.width = messageField.paintedWidth
                }
            }
        }

        // Message time information
        Text {
            id: metaStringField
            font.pixelSize: root.metaFontSize
            color: "white"
            text: metaString
            height: paintedHeight
            width: paintedWidth
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.bottomMargin: (!me ? 1 : 10)
        }
    }
}
