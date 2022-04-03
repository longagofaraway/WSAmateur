import QtQuick 2.12
import QtQuick.Controls 2.12

Item {
    property string statusText

    signal jsonAbility(string code, string pos)
    signal editAbility(string code, string pos)

    width: 150
    height: row.height + status.height + 5

    Row {
        id: row

    Column {
        id: cardSerial
        x: 10
        Text { text: "Enter card serial" }
        BasicTextInput {
            id: codeInput
            placeholderText: "BFR/S78-001"
        }
    }
    Column {
        id: abilityNum
        x: 10
        Text { text: "Ability number" }
        BasicTextInput {
            id: numberInput
            placeholderText: "1"
        }
    }
    }

    Text {
        id: status
        x: 10
        anchors { top: row.bottom; topMargin: 2 }
        text: statusText
    }

    Button {
        id: printButton
        anchors { left: row.right; leftMargin: 10 }
        text: "Print"
        onClicked: jsonAbility(codeInput.text, numberInput.text)
    }

    Button {
        id: editButton
        anchors { left: printButton.right; leftMargin: 10 }
        text: "Save"
        onClicked: editAbility(codeInput.text, numberInput.text)
    }

    function getCode() {
        return codeInput.text;
    }
    function getAbilityPos() {
        return numberInput.text;
    }
}
