import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Styles 1.4

import abilityComponent 1.0

AbilityComponent {
    id: abilityComponent

    signal abilitySizeChanged(qmlWidth: real, qmlHeight: real)

    function setTriggerText(triggerText: string) {
        abilityMenu.setTriggerText(triggerText);
    }

    function setAbilityType(newValue) {
        abilityCombo.dontTrigger = true;
        abilityCombo.currentIndex = abilityCombo.indexOfValue(newValue);
    }

    function setKeywords(keys) {
        for (let i = 0; i < keys.length; i++) {
            let index = keywords.indexOfValue(keys[i]);
            let object = keywords.model.get(index);
            object.selected = true;
        }
        keywords.updateText();
    }

    Component.onCompleted: {
        abilityMenu.passAbilityComponent(abilityComponent)
    }

    MouseArea {
        anchors.fill: parent
    }

    Rectangle {
        anchors.fill: parent
        color: "white"
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

            property bool dontTrigger: false
            currentIndex: -1
            textRole: "key"
            valueRole: "value"
            model: ListModel {
                ListElement { key: "Cont"; value: "Cont" }
                ListElement { key: "Auto"; value: "Auto" }
                ListElement { key: "Act"; value: "Act" }
                ListElement { key: "Event"; value: "Event" }
            }
            onCurrentIndexChanged: {
                activationTimes.enabled = (currentIndex == 1);
            }
            onCurrentValueChanged: {
                if (dontTrigger) {
                    dontTrigger = false;
                    return;
                }

                //effectTypeChanged(currentValue);
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
            id: keywords
            anchors.bottom: abilitySpecs.bottom

            textRole: "name"
            valueRole: "value"
            model: ListModel {
                ListElement { name: "CxCombo"; value: "Cxcombo"; selected: false }
                ListElement { name: "Brainstorm"; value: "Brainstorm"; selected: false }
                ListElement { name: "Backup"; value: "Backup"; selected: false }
                ListElement { name: "Encore"; value: "Encore"; selected: false }
                ListElement { name: "Assist"; value: "Assist"; selected: false }
                ListElement { name: "Alarm"; value: "Alarm"; selected: false }
                ListElement { name: "Experience"; value: "Experience"; selected: false }
                ListElement { name: "Resonance"; value: "Resonance"; selected: false }
                ListElement { name: "Bond"; value: "Bond"; selected: false }
                ListElement { name: "Change"; value: "Change"; selected: false }
                ListElement { name: "Memory"; value: "Memory"; selected: false }
                ListElement { name: "Replay"; value: "Replay"; selected: false }
            }
            onCheckedChanged: {
                abilityComponent.updateKeywords(keywordList);
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
        onAbilityMenuSizeChanged: {
            abilityComponent.abilitySizeChanged(qmlWidth, qmlHeight);
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
