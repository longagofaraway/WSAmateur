import QtQuick 2.12

import "menu"

Item {
    id: mlgn

    property int cardsSelected: 0
    property bool firstTurn: false

    signal finished()
    width: root.width
    height: root.height
    z: 1

    Text {
        id: textHeader
        anchors { top: parent.top; topMargin: parent.width * 0.1 }
        width: root.width
        text: firstTurn ? "You're going first" : "Your opponent goes first"
        color: "#F0F0F0"
        font.pointSize: 40
        horizontalAlignment: Text.AlignHCenter
    }
    Rectangle {
        id: delimiter
        anchors { top: textHeader.bottom; topMargin: 5 }
        width: root.width; height: 5
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0.0; color: "#00F0F0F0" }
            GradientStop { position: 0.3; color: "#00F0F0F0" }
            GradientStop { position: 0.34; color: "#F0F0F0" }
            GradientStop { position: 0.66; color: "#F0F0F0" }
            GradientStop { position: 0.7; color: "#00F0F0F0" }
            GradientStop { position: 1.0; color: "#00F0F0F0" }
        }
    }
    Text {
        anchors { top: delimiter.bottom; topMargin: 5 }
        width: root.width
        text: "Choose up to 5 cards to discard to waiting room."
        color: "#F0F0F0"
        font.pointSize: 18
        horizontalAlignment: Text.AlignHCenter
    }

    MainButton {
        id: button
        anchors.horizontalCenter: parent.horizontalCenter
        state: "active"

        mText: "Keep hand"
        y: mlgn.height * 0.7
        z: 5

        onClicked: mlgn.finished()
    }

    function cardSelected(selected) {
        cardsSelected += selected ? 1 : -1;
        if (cardsSelected) {
            button.setText("Discard " + String(cardsSelected));
        } else {
            button.setText("Keep hand");
        }
    }
}
