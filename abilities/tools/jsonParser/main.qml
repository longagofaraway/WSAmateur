import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 1.4

import jsonParser 1.0

Window {
    id: root
    visible: true
    width: 640
    height: 480
    title: qsTr("jsonParser")

    JsonParser {
        id: jsonParser
    }

    TextArea {
        id: textBox
        height: root.height *  0.7
        width: root.width * 0.8
        anchors.horizontalCenter: root.contentItem.horizontalCenter
        verticalAlignment: TextInput.AlignTop
        Component.onCompleted: {
            textBox.text = jsonParser.initialText();
        }
    }

    Button {
        id: btn
        anchors.top: textBox.bottom
        anchors.topMargin: 5
        anchors.horizontalCenter: root.contentItem.horizontalCenter
        text: "Parse"
        onClicked: {
            resultBox.text = jsonParser.createAbility(textBox.text);
        }
    }

    TextArea {
        id: resultBox
        anchors.top: btn.bottom
        anchors.topMargin: 5
        height: root.height * 0.2
        width: root.width * 0.8
        anchors.horizontalCenter: root.contentItem.horizontalCenter
        verticalAlignment: TextInput.AlignTop
    }
}
