import QtQuick 2.12
import QtQuick.Controls 2.12

Rectangle {
    id: numberQml
    property double fontSize: 8
    property string displayName
    signal numModifierChanged(string value)
    signal valueChanged(string value)

    function setNumMod(newValue) {
        combo.currentIndex = combo.indexOfValue(newValue);
    }
    function setValue(newValue) {
        textInput.text = newValue;
    }

    width: childrenRect.width
    height: childrenRect.height - 10

    Text {
        text: displayName
        anchors.bottom: numberQml.top
    }

Row {

    spacing: 5

    ComboBox {
        id: combo
        width: 70
        textRole: "key"
        valueRole: "value"
        model: ListModel {
            ListElement { key: "="; value: "ExactMatch" }
            ListElement { key: "<="; value: "UpTo" }
            ListElement { key: ">="; value: "AtLeast" }
        }
        onCurrentValueChanged: numberQml.numModifierChanged(currentValue);
    }

    Rectangle {
        width: 65
        height: combo.height
        border.color: "gray"
        border.width: 1

        TextInput {
            id: textInput
            anchors.fill: parent
            anchors.margins: 4
            verticalAlignment: TextInput.AlignVCenter
            onTextChanged: numberQml.valueChanged(text)
            selectByMouse: true
            font.pointSize: numberQml.fontSize

            property string placeholderText: "Enter value"

            Text {
                anchors.verticalCenter: textInput.verticalCenter
                text: textInput.placeholderText
                color: "#aaa"
                visible: !textInput.text
            }
        }
    }
}

}
