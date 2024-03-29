import QtQuick 2.12
import QtQuick.Controls 2.12

import "../basicTypes"

Rectangle {
    id: conditionImpl

    signal addCard()
    signal editCard()
    signal editCard2()
    signal editTarget()

    width: root.width

    MouseArea {
        anchors.fill: parent
    }

    Text {
        id: label
        anchors.horizontalCenter: conditionImpl.horizontalCenter
        text: "Condition Is Card"
        font.pointSize: 12
    }

    Row {
        spacing: 5
        anchors{ top: label.bottom; topMargin: 10 }
        anchors.horizontalCenter: conditionImpl.horizontalCenter

        Column {
            Text {
                text: "Target"
            }
            Button {
                text: "Open editor"
                onClicked: {
                    editTarget();
                }
            }
        }

        Column {
            id: card
            Text { text: "Card filter" }
            Button {
                text: "Open editor"
                onClicked: {
                    editCard();
                }
            }
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "+"
            }
            Button {
                property bool created: false
                text: "Add Card Filter"
                onClicked: {
                    if (!created) {
                        addCard();
                        text = "Open Editor";
                        created = true;
                    } else {
                        editCard2();
                    }
                }
            }
        }
    }
}
