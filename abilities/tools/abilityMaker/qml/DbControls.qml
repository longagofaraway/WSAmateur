import QtQuick 2.12
import QtQuick.Controls 2.12

import "basicTypes"

Item {
    signal addAbility(string code)
    signal popAbility(string code)

    signal setStatus(string status)
    onSetStatus: {
        statusText.text = status;
    }

    Column {
        id: cardSerial
        x: 10
        Text { text: "Enter card serial (i.e. BFR/S78-001)" }
        BasicTextInput {
            id: codeInput
        }
    }

    Button {
        id: addButton
        anchors { left: cardSerial.right; leftMargin: 10 }
        text: "Add ability to db"
        onClicked: addAbility(codeInput.text)
    }

    Button {
        id: removeButton
        anchors { left: addButton.right; leftMargin: 10 }
        text: "Remove last ability from db"
        onClicked: popAbility(codeInput.text)
    }

    Text {
        id: statusText
        x: 10
        anchors { top: addButton.bottom; topMargin: 5 }
    }
}
