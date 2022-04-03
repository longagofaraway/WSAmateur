import QtQuick 2.12
import QtQuick.Controls 2.12

Rectangle {
    id: effect

    signal effectTypeChanged(int index)
    signal componentReady()
    signal cancel()

    // incoming signals
    signal incomingEffectType(int index)
    onIncomingEffectType: {
        effectTypeCombo.currentIndex = index - 1;
    }

    property real effectImplY: effectTypeCombo.y + effectTypeCombo.height + 10

    x: 0
    y: 0
    width: root.width
    height: root.height - root.pathHeight - root.textAreaHeight - 10

    // Rectangle is transparent for mouse events, so we need this
    MouseArea {
        anchors.fill: parent
    }

    Text {
        id: typeLabel
        anchors.right: effectTypeCombo.left
        anchors.verticalCenter: effectTypeCombo.verticalCenter
        text: "Effect type:"
    }

    ComboBox {
        id: effectTypeCombo
        width: implicitWidth
        anchors.horizontalCenter: effect.horizontalCenter
        model: ["Attribute Gain", "Choose Card", "Reveal Card", "Move Card", "Search Card",
                "Pay Cost", "Ability Gain", "Move wr to deck", "Flip Over", "Backup",
                "Trigger Check Twice", "Look", "Non Mandatory", "Early Play", "Cannot Play",
                "Perform Effect", "Change State", "Deal Damage", "No Backups/Events", "Draw Card",
                "Swap Cards", "Cannot Attack", "Char auto can't deal damage", "Opp auto can't deal damage",
                "Can't Become Reversed", "Stock Swap", "Add Marker", "Bond", "Cannot Move", "Perform Replay",
                "Replay", "Side Attack Without Penalty", "Standby", "Shuffle", "Put On Stage Rested",
                "Remove Marker", "Cannot Stand"]
        currentIndex: -1
        onCurrentIndexChanged: {
            effectTypeChanged(currentIndex + 1);
        }
    }

    ConfirmButton {
        onClicked: {
            if (effectTypeCombo.currentIndex == -1)
                return;
            componentReady()
        }
    }

    CancelButton {
        onClicked: cancel()
    }
}
