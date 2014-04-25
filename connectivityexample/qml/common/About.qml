/**
 * Copyright (c) 2012-2014 Microsoft Mobile.
 */
import QtQuick 1.1

Flickable {
    id: root
    contentWidth: width
    contentHeight: text.height
    clip: true

    property int fontSize: 25

    Text {
        id: text
        anchors.centerIn: parent
        width: root.width - 20
        font.pixelSize: root.fontSize
        color: "white"
        wrapMode: Text.WordWrap
        onLinkActivated: {
            Qt.openUrlExternally(link);
        }
        text: "<h3>Connectivity Plug-in Example</h3>" +
              "<p>This application is an example demonstrating " +
              "the use of <a href=\"http://projects.developer.nokia.com/connectivityplugin\">Connectivity Plug-in</a>. " +
              "The application includes a chat to send and receive messages once a connection has been established.</p>" +
              "<p>This example application is hosted in " +
              "<a href=\"http://projects.developer.nokia.com/connectivityplugin\">" +
              "Nokia Developer Projects</a>.</p>"
    }
}
