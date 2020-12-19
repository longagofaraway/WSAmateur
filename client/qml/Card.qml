import QtQuick 2.12

Image {
    property string mSource: "cardback"

    width: root.width * 0.0677
    height: root.height * 0.1685
    mipmap: true
    smooth: true
    antialiasing: true
    source: "image://imgprov/" + mSource
}
