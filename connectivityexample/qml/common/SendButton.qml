/**
 * Copyright (c) 2012 Nokia Corporation.
 */

import QtQuick 1.1

Item {
    id: root
    //When button gets clicked
    signal clicked;

    width: parent.height
    height: parent.height

    Image {
        id: sendButtonImage
        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
        smooth: true
        source: "send-chat.png"
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: root.clicked();
    }
}
