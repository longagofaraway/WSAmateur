import QtQuick 2.12
import QtQuick.Controls 2.12

import '../Menus'
import '..'

Row {
    id: effectRow
    signal createEffect(string compId, string effect)
    signal setEffect(string compId, string effect)
    signal selectEffect(string compId)
    signal deleteEffect(string compId)

    signal createCondition(string compId, string condition)
    signal setCondition(string compId, string condition)
    signal selectCondition(string compId)
    signal deleteCondition(string compId)

    property string componentId
    property string effectMode: "createMode"
    property string conditionMode: "createMode"
    property string effectName
    property string conditionName
    property bool selected: false
    property bool conditionSelected: false

    spacing: 10
    Button {
        id: conditionButton
        width: 80
        height: 30
        highlighted: effectRow.conditionSelected
        text: {
            if (conditionMode === "createMode")
                return "+";
            else if (conditionMode === "selectMode")
                return conditionName;
            else
                return "";
        }
        onClicked: {
            if (conditionMode == "selectMode") {
                selectCondition(componentId);
            } else {
                conditionMenu.popup();
            }
        }

        /*CancelButton {
            anchors { horizontalCenter: conditionButton.right; verticalCenter: conditionButton.top }
            onClicked: deleteCondition(componentId)
            visible: conditionMode === "selectMode"
        }*/

        ConditionMenu {
            id: conditionMenu
            onMenuClicked: {
                if (conditionMode == "createMode")
                    createCondition(componentId, conditionId);
                else
                    setCondition(componentId, conditionId);
            }
            onAboutToShow: {
                conditionMenu.x = conditionButton.width;
                conditionMenu.y = conditionButton.height / 2 - conditionMenu.height / 2;
            }
        }
    }

    Button {
        id: effectButton
        width: 80
        height: 30
        highlighted: effectRow.selected
        text: {
            if (effectMode === "createMode")
                return "+";
            else if (effectMode == "selectMode")
                return effectName;
            else
                return "";
        }
        onClicked: {
            if (effectMode == "selectMode") {
                selectEffect(componentId);
            } else {
                effectMenu.popup();
            }
        }

        CancelButton {
            anchors { horizontalCenter: effectButton.right; verticalCenter: effectButton.top }
            onClicked: deleteEffect(componentId)
            visible: effectMode === "selectMode"
        }

        EffectMenu {
            id: effectMenu
            onMenuClicked: {
                if (effectMode == "createMode")
                    createEffect(componentId, effectId);
                else
                    setEffect(componentId, effectId);
            }
            onAboutToShow: {
                effectMenu.x = effectButton.width;
                effectMenu.y = effectButton.height / 2 - effectMenu.height / 2;
            }
        }
    }
}
