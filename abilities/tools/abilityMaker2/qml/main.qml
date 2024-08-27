import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Styles 1.4

//import abilityMaker 1.0

Window {
    id: window
    visible: true
    width: 780
    height: 580
    title: qsTr("Ability Maker")

    Rectangle {
        id: root
        anchors.fill: parent
    Row{
        anchors.verticalCenter: root.verticalCenter
        spacing: 10
    Column {
        id: rootCol
        spacing: 10

        ComboBox {
            id: rootCombo
            currentIndex: -1
            model: ["Cont", "Auto", "Act", "Event"]
        }
        Button {
            id: triggers
            width: rootCombo.width
            text: "Trigger"
            onClicked: highlighted = !highlighted
            onHighlightedChanged: {
                triggerCol.visible = highlighted;
            }
        }
    }
    Column {
        id: triggerCol
        ComboBox {
            id: triggerTypeCombo
            anchors.verticalCenter: rootCol.verticalCenter
            visible: false
            width: 200
            model: ["Zone change", "On play", "On state change", "On attack", "On backup of this",
                    "On trigger reveal", "Phase event", "End of this card's attack",
                    "On standby trigger effect", "On being attacked", "On damage cancel",
                    "On damage taken cancel", "On paying cost", "When 【ACT】 abillity used"]
            currentIndex: -1
            onCurrentIndexChanged: {

            }
        }
    }
    }
    }
}
