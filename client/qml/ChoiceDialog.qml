import QtQuick 2.15

import wsamateur 1.0

Rectangle {
    id: dialog

    property bool mLongtext: false
    property bool mCancelable: false
    property string mHeaderText: "Choose a card"
    property ChoiceDialogModel mModel: innerModel
    signal destroySignal()
    signal choiceMade(int choice)

    x: gGame.width / 2 - width / 2
    y: gGame.height / 2 - height / 2
    radius: 5
    border.width: 1
    color: "#F0564747"
    z: 500
    width: mLongtext ? 375 : (Math.max(200, header.contentWidth) + 25)
    height: header.contentHeight + lview.height + 11
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

    transitions: [
        Transition {
            from: "*"
            to: "active"
            NumberAnimation { properties: "opacity, scale"; duration: 200 }
        },
        Transition {
            from: "active"
            to: "*"
            SequentialAnimation {
                NumberAnimation { properties: "opacity, scale"; duration: 200 }
                ScriptAction { script: destroySignal() }
            }
        }
    ]

    MouseArea {
        x: -dialog.x
        y: -dialog.y
        width: gGame.width
        height: gGame.height
        z: -1
        hoverEnabled: true
        onClicked: {
            if (mCancelable)
                dialog.state = "";
        }
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

    Image {
        visible: mCancelable
        id: closeBtn
        anchors {
            right: dialog.right
            rightMargin: 5
            top: dialog.top
            topMargin: 5
        }
        source: "qrc:///resources/images/closeButton"

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onEntered: closeBtn.source = "qrc:///resources/images/closeButtonHighlighted"
            onExited: closeBtn.source = "qrc:///resources/images/closeButton"
            onClicked: dialog.state = "";
        }
    }

    ListView {
        id: lview
        anchors.top: header.bottom
        anchors.topMargin: 3
        anchors.horizontalCenter: parent.horizontalCenter
        width: dialog.width - 25
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
                width: parent.width
                text: model.text
                font.pointSize: dialog.mLongtext ? 10 : 18
                wrapMode: Text.WordWrap
                textFormat: Text.StyledText
                horizontalAlignment: dialog.mLongtext ? Text.AlignJustify : Text.AlignHCenter
                leftPadding: 4
                rightPadding: 4
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
                    choiceMade(model.index);
                }
            }
        }
    }
}
