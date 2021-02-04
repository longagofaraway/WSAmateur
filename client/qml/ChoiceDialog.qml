import QtQuick 2.15

import wsamateur.choiceDialogModel 1.0

Rectangle {
    id: dialog

    property string mHeaderText: "Choose a card"
    property ChoiceDialogModel mModel: innerModel

    x: gGame.width / 2 - width / 2
    y: gGame.height / 2 - height / 2
    radius: 5
    border.width: 1
    color: "#F0564747"
    z: 160
    width: Math.max(200, header.contentWidth) + 25
    height: childrenRect.height + 11
    opacity: 0
    scale: 0

    states: State {
        name: "active"
        PropertyChanges {
            target: dialog
            opacity: 1
            scale: 1
        }
    }

    transitions: Transition {
        NumberAnimation { properties: "opacity, scale"; duration: 200 }
    }

    Text {
        id: header
        anchors {
            horizontalCenter: parent.horizontalCenter
            top: dialog.top
            topMargin: 3
        }
        text: mHeaderText
        font.family: "Futura Bk BT"
        font.pointSize: 22
        color: "white"
    }

    ListView {
        id: lview
        anchors.top: header.bottom
        anchors.topMargin: 3
        anchors.horizontalCenter: parent.horizontalCenter
        width: 200
        height: contentHeight
        interactive: false

        model: mModel

        delegate: Rectangle {
            id: delegateRect
            height: choiceText.contentHeight + 10
            width: ListView.view.width
            radius: 8
            border.width: 1
            color: "#F0F0F0"

            Text {
                id: choiceText
                anchors.centerIn: parent
                text: model.text
                font.pointSize: 18
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true

                onEntered: {
                    delegateRect.border.width = 3;
                    delegateRect.border.color = "#BBBBBB";
                }
                onExited: {
                    delegateRect.border.width = 1;
                    delegateRect.border.color = "black";
                }
                onPressed: {
                    delegateRect.color = "#BBBBBB";
                    delegateRect.border.color = "white";
                }
                onReleased: {
                    delegateRect.color = "#F0F0F0";
                    delegateRect.border.color = "#BBBBBB";
                }
                onClicked: {
                    dialog.state = "";
                    gGame.pause(200);
                    gGame.getPlayer().sendChoice(model.index);
                }
            }
        }
    }
}
