import QtQuick 2.12
import QtQuick.Controls 2.12

import "../basicTypes"

Rectangle {
    id: triggerImpl

    signal phaseStateChanged(int index)
    signal phaseChanged(int index)
    signal ownerChanged(int index)

    width: root.width

    MouseArea {
        anchors.fill: parent
    }

    Text {
        id: label
        anchors.horizontalCenter: triggerImpl.horizontalCenter
        text: "Phase Trigger"
        font.pointSize: 12
    }

    Row {
        spacing: 5
        anchors{ top: label.bottom; topMargin: 10 }
        anchors.horizontalCenter: triggerImpl.horizontalCenter

        Column {
            Text { text: "At the" }
            PhaseState {
                id: phaseStateCombo
                Component.onCompleted: {
                    valueChanged.connect(phaseStateChanged);
                }
            }
        }
        Column {
            Text { text: "Phase" }
            Phase {
                id: phaseCombo
                Component.onCompleted: {
                    valueChanged.connect(phaseChanged);
                }
            }
        }
        Column {
            Text { text: "Active player" }
            Player {
                id: playerCombo
                Component.onCompleted: {
                    valueChanged.connect(ownerChanged);
                }
            }
        }
    }

    function setPhase(value) {
        phaseCombo.setValue(value);
    }

    function setPhaseState(value) {
        phaseStateCombo.setValue(value);
    }

    function setPlayer(value) {
        playerCombo.setValue(value);
    }
}
