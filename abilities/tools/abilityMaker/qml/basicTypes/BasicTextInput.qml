import QtQuick 2.12

Rectangle {
    id: basicTextInput
    signal valueChanged(string value)

    property string text: textInput.text
    property string placeholderText: "Enter value"

    width: 100
    height: 30
    border.color: "gray"
    border.width: 1

    TextInput {
        id: textInput
        anchors.fill: parent
        anchors.margins: 4
        verticalAlignment: TextInput.AlignVCenter
        selectByMouse: true

        Text {
            anchors.verticalCenter: textInput.verticalCenter
            text: basicTextInput.placeholderText
            color: "#aaa"
            visible: !textInput.text
        }

        onTextEdited: {
            valueChanged(text)
        }
    }

    function setValue(value) {
        textInput.text = value;
    }
}
