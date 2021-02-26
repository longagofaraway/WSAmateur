import QtQuick 2.15
import QtGraphicalEffects 1.12

Rectangle {
    id: activatedAbility

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
                    font.pointSize: 16
                    font.family: "Futura Bk BT"
                }

                AbilityButton {
                    isPlayButton: true
                    anchors.horizontalCenter: controlPanel.horizontalCenter
                    y: model.cancelBtnActive ? (header.contentHeight + 4) :
                        ((controlPanel.height + header.contentHeight - height - 8) / 2)
                    width: controlPanel.width * 0.8
                }
                AbilityButton {
                    isPlayButton: false
                    anchors.horizontalCenter: controlPanel.horizontalCenter
                    y: model.playBtnActive ? (header.contentHeight + height + 12) :
                        ((controlPanel.height + header.contentHeight - height - 8) / 2)
                    width: controlPanel.width * 0.8
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
                textFormat: Text.StyledText
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
