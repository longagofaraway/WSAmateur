import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 1.4

import jsonParser 1.0

Window {
    id: root
    visible: true
    width: 640
    height: 580
    title: qsTr("jsonParser")

    JsonParser {
        id: jsonParser
    }

    Column {
        id: col
        anchors.horizontalCenter: root.contentItem.horizontalCenter
    TextArea {
        id: textBox
        height: 250
        width: root.width * 0.8
        verticalAlignment: TextInput.AlignTop
        Component.onCompleted: {
            textBox.text = jsonParser.initialText();
        }
    }

    Button {
        id: btn
        anchors.horizontalCenter: col.horizontalCenter
        text: "Parse"
        onClicked: {
            resultBox.text = jsonParser.createAbility(textBox.text);
            hexBox.text = jsonParser.printEncodedAbility();
            fromBinBox.text = jsonParser.printDecodedAbility();
        }
    }

    TextArea {
        id: resultBox
        height: 90
        width: root.width * 0.8
        verticalAlignment: TextInput.AlignTop
    }
    TextArea {
        id: hexBox
        height: 90
        width: root.width * 0.8
        verticalAlignment: TextInput.AlignTop
    }
    TextArea {
        id: fromBinBox
        height: 50
        width: root.width * 0.8
        verticalAlignment: TextInput.AlignTop
    }
    DbControls {
        id: dbControls
        anchors { top: fromBinBox.bottom; topMargin: 10 }
        onAddAbility: {
            statusText = jsonParser.addToDb(getCode(), textBox.text);
        }
        onPopAbility: {
            statusText = jsonParser.popFromDb(getCode());
        }
    }
    }
}
