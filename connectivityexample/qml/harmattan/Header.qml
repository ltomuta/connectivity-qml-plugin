/**
 * Copyright (c) 2012-2014 Microsoft Mobile.
 */

import QtQuick 1.1
import com.nokia.meego 1.0

Rectangle {
    property alias text: label.text

    anchors {
        top: parent.top
        left: parent.left
        right: parent.right
    }
    color: theme.inverted ? "black" : "white"
    height: 40

    Rectangle {
        id: border
        width: parent.width
        height: 2
        anchors.bottom: parent.bottom
        z: parent.z + 1
        color: "#444444"
    }

    Label {
        id: label
        anchors {
            fill: parent
            rightMargin: 10
        }
        color: theme.inverted ? "white" : "black"
        horizontalAlignment: Text.AlignRight
        verticalAlignment: Text.AlignVCenter
    }
}
