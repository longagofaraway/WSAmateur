import QtQuick 2.12

Rectangle {
    id: basicTextInput

    property string text: textInput.text

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
        focus: true

        property string placeholderText: "Enter value"

        Text {
            anchors.verticalCenter: textInput.verticalCenter
            text: textInput.placeholderText
            color: "#aaa"
            visible: !textInput.text
        }
    }

    function clear() {
        textInput.text = "";
        basicTextInput.border.color = "grey";
    }

    function setError() {
        basicTextInput.border.color = "red";
    }
}
