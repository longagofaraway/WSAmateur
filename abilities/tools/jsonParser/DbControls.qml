import QtQuick 2.12
import QtQuick.Controls 2.12

Item {
    property string statusText

    signal addAbility(string code)
    signal popAbility(string code)

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
        x: 10
        anchors { top: addButton.bottom; topMargin: 5 }
        text: statusText
    }

    function getCode() {
        return codeInput.text;
    }
}
