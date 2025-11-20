import QtQuick 2.12
import QtQuick.Controls 2.12

Item {
    id: itemRoot
    property string componentName
    signal valueChanged(string newValue)

    function setValue(newValue) {
        textInput.text = newValue;
    }

    Column {
        id: column
        spacing: 5

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
                    valueChanged(text)
                }
            }
        }
    }
}
