import QtQuick 2.0

Item {
    id: warningWindow

    property var cards: []

    MouseArea {
        anchors.fill: parent
        z: -1
        hoverEnabled: true
    }

    Rectangle {
        id: mainRectangle

        anchors.centerIn: parent

        width: mainWindow.width / 2.7
        height: mainColumn.implicitHeight * 1.2
        border.width: 2
        border.color: "white"
        color: "#E0000000"
        radius: 10

        Column {
            id: mainColumn
            anchors.centerIn: parent
            spacing: 20

            Text {
                anchors {
                    horizontalCenter: parent.horizontalCenter
                }
                text: "The following cards are not supported:"
                color: "white"
                font.pointSize: 24
            }

            Text {
                anchors {
                    horizontalCenter: parent.horizontalCenter
                }
                text: {
                    console.log(cards);
                    const n = cards.length;
                    let newText = warningWindow.cards.slice(0,4).join('\n');
                    if (n > 4)
                        newText += "\n...";
                    return newText;
                }
                color: "white"
                font.pointSize: 18
                horizontalAlignment: Text.AlignHCenter
            }

            MenuButton {
                anchors {
                    horizontalCenter: parent.horizontalCenter
                }

                text: "OK"
                onPressed: {
                    warningWindow.destroy();
                }
            }
        }
    }
}
