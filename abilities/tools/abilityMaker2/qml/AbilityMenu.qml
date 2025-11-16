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

    function setWorkingArea(workingArea) {
        effectsTree.setWorkingArea(workingArea);
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
            highlighted = !highlighted
            if (!scrollView.triggerSet) {
                triggerMenu.popup()
            } else {
                scrollView.openTrigger();
            }
        }

        TriggerMenu {
            id: triggerMenu
            onAboutToHide: triggers.highlighted = false
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

    EffectsTree {

        id: effectsTree
        Text {
            text:"Lol!"
        }
    }

    /*
    Button {
        id: condition
        text: "Condition"
        onClicked: {
            highlighted = !highlighted
            conditionMenu.popup()
        }

        ConditionMenu {
            id: conditionMenu
            onAboutToHide: condition.highlighted = false
            onAboutToShow: {
                conditionMenu.x = condition.x + condition.width;
                conditionMenu.y = condition.height / 2 - conditionMenu.height / 2;
            }
        }
    }

    Button {
        id: effect
        text: "Effect"
        onClicked: {
            highlighted = !highlighted
            effectMenu.popup()
        }

        EffectMenu {
            id: effectMenu
            onAboutToHide: effect.highlighted = false
            onAboutToShow: {
                effectMenu.x = effect.x + effect.width;
                effectMenu.y = effect.height / 2 - effectMenu.height / 2;
            }
        }
    }*/
}
}
