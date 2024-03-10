import QtQuick 2.15
import QtGraphicalEffects 1.15

Rectangle {
    id: btnRect

    property bool isPlayButton
    property color btnColor: "#4A5366"
    property bool active: isPlayButton ? model.playBtnActive : model.cancelBtnActive
    property string mText: isPlayButton ? model.playBtnText : model.cancelBtnText

    height: 30
    radius: 20
    color: btnColor
    visible: active
    layer.enabled: false
    layer.effect: Glow {
        samples: 12
        color: "#FCDE01"
    }

    Text {
        id: btnText
        anchors.centerIn: parent
        text: mText
        color: "white"
        font.family: "Futura Bk BT"
        font.pointSize: 16
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        propagateComposedEvents: true
        onEntered: { if (!active) return; btnRect.layer.enabled = true; }
        onExited: { if (!active) return; btnRect.layer.enabled = false; }
        onPressed: { if (!active) return; btnRect.color = "white"; btnText.color = btnColor; }
        onReleased: { if (!active) return; btnRect.color = btnColor; btnText.color = "white"; }
        onClicked: {
            if (!active) return;
            if (isPlayButton) gGame.getPlayer().playAbility(model.index);
            else gGame.getPlayer().cancelAbility(model.index);
        }
    }
}
