import QtQuick 2.12
import QtQuick.Controls 1.1

Item {
    Button {
        text: "start"
        onClicked: {
            mainLoader.source = "GameWindow.qml";
        }
    }
}
