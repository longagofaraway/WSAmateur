import QtQuick 2.12
import QtQuick.Controls 2.12

Rectangle {
    id: specifier
    width: 70
    height: 40

    signal removeCardSpecifier()

    CancelButton {
        anchors.top: specifier.top
        anchors.left: specifier.right
        anchors.leftMargin: 10

        onClicked: removeCardSpecifier()
    }
}
