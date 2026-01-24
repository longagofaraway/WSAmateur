import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Styles 1.4

import 'Menus'
import effectsTree 1.0

ScrollView {
    id: scrollView
    width: rootCol.width
    clip: true

    property bool triggerSet: false
    property bool costSet: false

    signal createTrigger(triggerId: string)
    signal openTrigger()
    signal abilityMenuSizeChanged(qmlWidth: real, qmlHeight: real)

    function setWorkingArea(workingArea) {
        effectsTree.setWorkingArea(workingArea);
    }
    function passAbilityComponent(abilityComponent) {
        effectsTree.setAbilityComponent(abilityComponent);
    }
    function setTriggerText(triggerText: string) {
        triggers.text = 'Trigger' + '\n' + triggerText
    }

Column {
    id: rootCol
    spacing: 10


    Button {
        id: triggers
        text: "Trigger"
        onClicked: {
            highlighted = true;
            effectsTree.loseFocus();
            if (!scrollView.triggerSet) {
                triggerMenu.popup()
            } else {
                scrollView.openTrigger();
            }
        }

        TriggerMenu {
            id: triggerMenu
            onAboutToShow: {
                triggerMenu.x = triggers.x + triggers.width;
                triggerMenu.y = triggers.height / 2 - triggerMenu.height / 2;
            }
            onCreateTrigger: {
                scrollView.createTrigger(triggerId);
                triggerSet = true;
            }
        }
    }

    Button {
        id: cost
        text: "Cost"
        onClicked: {
            highlighted = !highlighted
            costMenu.popup()
        }

        CostMenu {
            id: costMenu
            onAboutToHide: cost.highlighted = false
            onAboutToShow: {
                costMenu.x = cost.x + cost.width;
                costMenu.y = cost.height / 2 - costMenu.height / 2;
            }
        }
    }

    Rectangle {
        id: delimiter
        height: 2
        width: effectsTree.width
        border.color: "grey"
    }

    Row {
        spacing: 41
        Text {
            text: "Conditions"
        }
        Text {
            text: "Effects"
        }
    }

    Connections {
        target: effectsTree
        function onGotFocus() {
            triggers.highlighted = false;
        }
        function onSizeChanged(qmlWidth, qmlHeight) {
            abilityMenuSizeChanged(qmlWidth, qmlHeight);
        }
    }

    EffectsTree {
        id: effectsTree
    }
}
}
