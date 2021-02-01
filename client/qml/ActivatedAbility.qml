import QtQuick 2.15
import QtGraphicalEffects 1.12

Rectangle {
    id: activatedAbility

    property color btnColor: "#4A5366"

    color: "#00000000"
    width: 300
    height: cardImg.height + effectText.contentHeight - 5
    layer.enabled: model.active
    layer.effect: Glow {
        samples: 12
        color: "#FCDE01"
    }

    Column {
        id: col
        spacing: -5
        Row {
            Card {
                id: cardImg
                height: 110
                verticalAlignment: Image.AlignTop
                fillMode: Image.PreserveAspectCrop
                clip: true
                mSource: model.code
            }
            Image {
                id: controlPanel
                width: activatedAbility.width - cardImg.width
                height: cardImg.height
                source: "qrc:///resources/images/abbg"

                Text {
                    id: header
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "Ability"
                    font.pointSize: 20
                    font.family: "Futura Bk BT"
                }

                Rectangle {
                    id: btnRect
                    anchors.horizontalCenter: controlPanel.horizontalCenter
                    y: (controlPanel.height + header.contentHeight - height - 8) / 2
                    width: controlPanel.width * 0.8
                    height: 30
                    radius: 20
                    color: btnColor
                    visible: model.btnActive
                    layer.enabled: false
                    layer.effect: Glow {
                        samples: 12
                        color: "#FCDE01"
                    }

                    Text {
                        id: btnText
                        anchors.centerIn: parent
                        text: model.btnText
                        color: "white"
                        font.family: "Futura Bk BT"
                        font.pointSize: 16
                    }

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        onEntered: { if (!model.btnActive) return; btnRect.layer.enabled = true; }
                        onExited: { if (!model.btnActive) return; btnRect.layer.enabled = false; }
                        onPressed: { if (!model.btnActive) return; btnRect.color = "white"; btnText.color = btnColor; }
                        onReleased: { if (!model.btnActive) return; btnRect.color = btnColor; btnText.color = "white"; }
                    }
                }
            }
        }
        Rectangle {
            color: "#FFFFFF"
            radius: 5
            border.width: 1
            width: activatedAbility.width
            height: effectText.contentHeight

            Text {
                id: effectText
                width: parent.width
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignJustify
                leftPadding: 4
                rightPadding: 4
                text: model.text
                font.family: "Futura Bk BT"
                font.pointSize: 10
            }
        }
    }
}
