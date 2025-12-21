import QtQuick 2.12
import QtQuick.Controls 2.12

Rectangle {
    id: place

    property string displayName: 'Place'

    width: childrenRect.width
    height: childrenRect.height

    Text {
        id: header
        x: 40
        font.pointSize: 12
        text: place.displayName
        anchors.bottom: place.top
        anchors.bottomMargin: 10
    }
}
