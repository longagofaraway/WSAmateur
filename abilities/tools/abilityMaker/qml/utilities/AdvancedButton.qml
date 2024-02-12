import QtQuick 2.12
import QtQuick.Controls 2.12


Button {
    id: button
    signal rightClicked

    MouseArea{
        id: buttonMouseArea

        acceptedButtons: Qt.RightButton
        anchors.fill: parent;
        onClicked: {
            button.rightClicked();
        }
    }
}
