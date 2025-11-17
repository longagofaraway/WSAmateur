import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Styles 1.4

import abilityComponent 1.0

AbilityComponent {
    id: abilityComponent

    function setTriggerText(triggerText: string) {
        abilityMenu.setTriggerText(triggerText);
    }

    Component.onCompleted: {
        abilityMenu.passAbilityComponent(abilityComponent)
    }

    Column {
        id: abilityTypeCombo
        anchors.horizontalCenter: abilityComponent.horizontalCenter
        Text {
            anchors.horizontalCenter: abilityCombo.horizontalCenter
            text: "Ability type:"
            font.pointSize: 8
        }
        ComboBox {
            id: abilityCombo
            currentIndex: -1
            model: ["Cont", "Auto", "Act", "Event"]
            onCurrentIndexChanged: {
                activationTimes.enabled = (currentIndex == 1);
            }
        }
    }

    Row {
        id: abilitySpecs
        anchors { top: abilityTypeCombo.bottom; topMargin: 20; horizontalCenter: abilityComponent.horizontalCenter }
        spacing: 20
        Column {
            id: activationTimes

            enabled: false

            Text {
                text: "Activates up to:"
            }

            ComboBox {
                model: ["always", "1", "2"]
            }
        }

        MultiselectComboBox {
            anchors.bottom: abilitySpecs.bottom
            model: ListModel {
                ListElement { name: "CxCombo"; selected: false }
                ListElement { name: "Brainstorm"; selected: false }
                ListElement { name: "Backup"; selected: false }
                ListElement { name: "Encore"; selected: false }
                ListElement { name: "Assist"; selected: false }
                ListElement { name: "Alarm"; selected: false }
                ListElement { name: "Experience"; selected: false }
                ListElement { name: "Resonance"; selected: false }
                ListElement { name: "Bond"; selected: false }
                ListElement { name: "Change"; selected: false }
                ListElement { name: "Memory"; selected: false }
                ListElement { name: "Replay"; selected: false }
            }
        }
    }

    AbilityMenu {
        id: abilityMenu
        anchors { top: abilitySpecs.bottom; topMargin: 10; bottom: abilityComponent.bottom }

        onCreateTrigger: {
            abilityComponent.createTrigger(triggerId, workingArea);
        }
        onOpenTrigger: {
            abilityComponent.openTrigger(workingArea);
        }
    }

    WorkingArea {
        id: workingArea
        anchors { top: abilityMenu.top; bottom: abilityComponent.bottom; left: abilityMenu.right; right: abilityComponent.right }
        Component.onCompleted: {
            abilityMenu.setWorkingArea(workingArea)
        }
    }
}
