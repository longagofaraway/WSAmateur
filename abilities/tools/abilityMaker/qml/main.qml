import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Styles 1.4

import abilityMaker 1.0

Window {
    visible: true
    width: 640
    height: 580
    title: qsTr("Ability Maker")


    AbilityMaker {
        id: root

        property real pathHeight: 32
        property real textAreaHeight: 80

        anchors.fill: parent

        Rectangle {
            id: path
            anchors { top: parent.top; left: parent.left; right: parent.right; leftMargin: 5 }
            height: 30

            Text {
                anchors { top: parent.top; left: parent.left }
                text: "/ability"
            }
        }

        Text {
            id: description
            anchors { bottom: parent.bottom; left: parent.left; right: parent.right; leftMargin: 5; bottomMargin: 5; rightMargin: 5 }
            height: root.textAreaHeight
            wrapMode: Text.WordWrap
            textFormat: Text.StyledText
            horizontalAlignment: Text.AlignJustify
            font.family: "Futura Bk BT"
            font.pointSize: 10
        }

        function setDescription(descr) {
            description.text = descr;
        }
    }
}
