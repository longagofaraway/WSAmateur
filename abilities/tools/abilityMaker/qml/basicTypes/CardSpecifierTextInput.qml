import QtQuick 2.12

Rectangle {
    property int position
    signal valueChanged(int pos, string value)

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

        property string placeholderText: "Enter value"

        Text {
            anchors.verticalCenter: textInput.verticalCenter
            text: textInput.placeholderText
            color: "#aaa"
            visible: !textInput.text
        }

        onTextEdited: {
            valueChanged(position, text)
        }
    }

    function setValue(value) {
        textInput.text = value;
    }
}
