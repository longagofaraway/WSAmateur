import QtQuick 2.12
import QtQuick.Controls 2.12

import "utilities"

Rectangle {
    id: ability

    property bool noButtons: false

    signal componentReady()
    signal cancel()
    signal setAbilityType(int type)
    signal setActivationTimes(int times)
    signal setKeywords(var keywords)
    signal editTriggers()
    signal editCost()
    signal editEffects()
    signal setTrigger(triggerName: string)
    signal addCost(costName: string)
    signal templateChanged(int index)

    width: root.width
    height: root.height - root.pathHeight - root.textAreaHeight - 10

    MouseArea {
        anchors.fill: parent
    }

    Text {
        anchors.right: rootCombo.left
        anchors.verticalCenter: rootCombo.verticalCenter
        text: "Ability type:"
        font.pointSize: 8
    }

    ComboBox {
        id: rootCombo
        anchors.horizontalCenter: ability.horizontalCenter
        currentIndex: -1
        model: ["Cont", "Auto", "Act", "Event"]
        onCurrentIndexChanged: {
            ability.setAbilityType(currentIndex + 1);
            let ab = "";
            switch (currentIndex) {
            case 0:
                ab = "ContAbility";
                activationTimes.enabled = false;
                keywords.enabled = true;
                trigger.enabled = false;
                cost.enabled = false;
                effects.enabled = true;
                break;
            case 1:
                ab = "AutoAbility";
                activationTimes.enabled = true;
                keywords.enabled = true;
                trigger.enabled = true;
                cost.enabled = true;
                effects.enabled = true;
                break;
            case 2:
                ab = "ActAbility";
                activationTimes.enabled = false;
                keywords.enabled = true;
                trigger.enabled = false;
                cost.enabled = true;
                effects.enabled = true;
                break;
            case 3:
                ab = "EventAbility";
                activationTimes.enabled = false;
                keywords.enabled = true;
                trigger.enabled = false;
                cost.enabled = false;
                effects.enabled = true;
                break;
            }
        }
    }

    ComboBox {
        id: templates
        anchors.right: ability.right
        currentIndex: -1
        model: ["Brainstorm wr", "Brainstorm draw", "Encore", "Bond"]
        onCurrentIndexChanged: {
            ability.templateChanged(currentIndex);
        }
    }

    Column {
        id: activationTimes

        x: 40
        anchors { top: rootCombo.bottom; topMargin: 10; }
        enabled: false

        Text {
            text: "Activates up to:"
        }

        ComboBox {
            model: ["always", "1", "2"]
            onCurrentIndexChanged: {
                ability.setActivationTimes(currentIndex);
            }
        }
    }

    Keywords {
        id: keywords
        anchors { top: rootCombo.bottom; left: activationTimes.right; topMargin: 10; leftMargin: 10 }

        Component.onCompleted: {
            keywords.selected.connect(ability.setKeywords);
        }
    }

    TriggerColumn {
        id: trigger

        anchors { top: rootCombo.bottom; left: keywords.right; topMargin: 10; leftMargin: 10 }

        Component.onCompleted: {
            trigger.editTriggers.connect(ability.editTriggers);
            trigger.setTrigger.connect(ability.setTrigger);
        }
    }

    CostColumn {
        id: cost

        anchors { top: rootCombo.bottom; left: trigger.right; topMargin: 10; leftMargin: 10 }

        Component.onCompleted: {
            cost.editCost.connect(ability.editCost);
            cost.addCost.connect(ability.addCost);
        }
    }

    Column {
        id: effects

        anchors { horizontalCenter: ability.horizontalCenter; top: keywords.bottom; topMargin: 25 }
        enabled: false

        Text {
            text: "Effects:"
        }

        Button {
            text: "Open editor"
            onClicked: {
                editEffects();
            }
        }
    }

    ConfirmButton {
        visible: !noButtons
        onClicked: componentReady()
    }

    CancelButton {
        visible: !noButtons
        onClicked: cancel()
    }

    function setActualY() {
        y = -mapToItem(root, 0, 0).y + root.pathHeight;
    }

    function fixAbilityType(value) {
        rootCombo.currentIndex = value - 1;
        rootCombo.enabled = false;
    }

    function changeAbilityType(value) {
        rootCombo.currentIndex = value - 1;
    }

    function changeKeywords(keywordsString) {
        keywords.setKeywords(keywordsString);
    }
}
