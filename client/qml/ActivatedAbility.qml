import QtQuick 2.15
import QtGraphicalEffects 1.12

Item {
    id: activatedAbility

    property string mSource
    property string mAbilityText
    property string mBtnText: "Cancel"
    property bool mBtnActive: true

    width: 300
    height: cardImg.height + effectText.contentHeight

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
                mSource: activatedAbility.mSource
            }
            Rectangle {
                id: controlPanel
                width: activatedAbility.width - cardImg.width
                height: cardImg.height
                color: "#80000000"

                Rectangle {
                    id: btnRect
                    anchors.centerIn: parent
                    width: controlPanel.width * 0.8
                    height: 30
                    radius: 20
                    color: activatedAbility.mBtnActive ? "#FFFFFF" : "#929292"
                    visible: activatedAbility.mBtnActive
                    layer.enabled: false
                    layer.effect: Glow {
                        id: btnGlow
                        samples: 12
                        color: "#FCDE01"
                        visible: false
                    }

                    Text {
                        id: btnText
                        anchors.centerIn: parent
                        text: mBtnText
                        font.family: "Futura Bk BT"
                        font.pointSize: 16
                    }

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        onEntered: { if (!mBtnActive) return; btnRect.layer.enabled = true; }
                        onExited: { if (!mBtnActive) return; btnRect.layer.enabled = false; }
                        onPressed: { if (!mBtnActive) return; btnRect.color = "#3D3C3A"; btnText.color = "white"; }
                        onReleased: { if (!mBtnActive) return; btnRect.color = "#FFFFFF"; btnText.color = "black"; }
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
                text: mAbilityText
                //text: "AUTO [(1) Put the top card of your deck in your clock & put a card from your hand into your waiting room] When this card is put into your waiting room from the stage, you may pay cost. If you do, look at up to six cards from the top of your deck, search for up to 1 red character and up to 1 blue character, reveal them to your opponent, put them into your hand, and put the rest into your waiting room."
                //text: "You can't take damage from your opponent character's AUTO effects."
                font.family: "Futura Bk BT"
                font.pointSize: 10
            }
        }
    }
    /*Card {
        anchors.top: col.bottom
        width: activatedAbility.width
        height: 200
        verticalAlignment: Image.AlignTop
        fillMode: Image.PreserveAspectCrop
        clip: true
        mSource: "KGL/S79-055"

        Rectangle {
            //color: "#F2FFFE"
            color: "#90FFFFFF"
            radius: 5
            border.width: 1
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: effectText2.contentHeight
            Text {
                id: effectText2
                width: parent.width
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignJustify
                leftPadding: 4
                rightPadding: 4
                text: "AUTO [(1) Put the top card of your deck in your clock & put a card from your hand into your waiting room] When this card is put into your waiting room from the stage, you may pay cost. If you do, look at up to six cards from the top of your deck, search for up to 1 red character and up to 1 blue character, reveal them to your opponent, put them into your hand, and put the rest into your waiting room."
                font.family: "Futura Bk BT"
                font.pointSize: 10
            }
        }
    }*/
}
