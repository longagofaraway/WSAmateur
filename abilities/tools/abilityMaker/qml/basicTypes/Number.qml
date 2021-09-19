import QtQuick 2.12
import QtQuick.Controls 2.12

Row {
    signal numModifierChanged(int index)
    signal valueChanged(string value)

    signal incomingValue(string value)
    signal incomingNumMod(int index)

    onIncomingValue: {
        textInput.text = value;
    }
    onIncomingNumMod: {
        combo.currentIndex = index - 1;
    }

    spacing: 5

    ComboBox {
        id: combo
        width: 70
        model: ["=", "<=", ">="]
        onCurrentIndexChanged: numModifierChanged(currentIndex + 1)
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
            onTextChanged: valueChanged(text)
            selectByMouse: true

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
