import QtQuick 2.12
import QtQuick.Controls 2.12

Item {
    id: itemRoot

    property string componentName
    signal valueChanged(string newValue, string compId)

    width: childrenRect.width

    function setValue(newValue, compId) {
        if (componentId !== compId)
            return;
        textInput.text = newValue;
    }
    Connections {
        target: parentComponent

        function onSetString(newValue, compId) {
            setValue(newValue, compId);
        }
        function onSetInt32(newValue, compId) {
            setValue(newValue.toString(), compId);
        }
        function onSetInt8(newValue, compId) {
            setValue(newValue.toString(), compId);
        }
        function onSetUInt8(newValue, compId) {
            setValue(newValue.toString(), compId);
        }
    }

    Column {
        id: column
        spacing: 5
        width: childrenRect.width

        Text {
            text: itemRoot.componentName
            font.pointSize: 8
            anchors.horizontalCenter: column.horizontalCenter
        }

        Rectangle {
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
                    valueChanged(text, componentId)
                }
            }
        }
    }
}
