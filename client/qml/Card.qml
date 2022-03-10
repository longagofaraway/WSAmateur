import QtQuick 2.12

Item {
    id: cardImage

    property string mSource: "cardback"
    property int verticalAlignment: Image.AlignVCenter
    property int fillMode: Image.Stretch

    width: root.width * 0.0677
    height: root.height * 0.1685

    Image {
        property bool isVertical: implicitHeight > implicitWidth

        anchors.centerIn: parent
        width: isVertical ? cardImage.width : cardImage.height
        height: isVertical ? cardImage.height : cardImage.width
        rotation: isVertical ? 0 : 90
        mipmap: true
        smooth: true
        antialiasing: true
        source: "image://imgprov/" + mSource

        verticalAlignment: cardImage.verticalAlignment
        fillMode: cardImage.fillMode
    }
}
