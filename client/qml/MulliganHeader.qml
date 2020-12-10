import QtQuick 2.12

Item {
    id: mlgn

    property int cardsSelected: 0

    signal finished()
    width: root.width
    y: 170
    z: 1

    Text {
        id: textHeader
        width: root.width
        text: "Mulligan"
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

        mText: "Keep hand"
        y: 600

        onClicked: mlgn.finished()
    }

    function cardSelected(selected) {
        cardsSelected += selected ? 1 : -1;
        if (cardsSelected)
            button.setText("Discard " + String(cardsSelected));
        else
            button.setText("Keep hand");
    }
}
