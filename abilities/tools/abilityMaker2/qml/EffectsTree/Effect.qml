import QtQuick 2.12
import QtQuick.Controls 2.12

import '../Menus'
import '..'

Row {
    id: effectRow
    signal createEffect(string compId, string effect)
    signal selectEffect(string compId)
    signal deleteEffect(string compId)

    property string componentId
    property string effectMode: "createMode"
    property string effectName
    property string conditionName
    property bool selected: false

    spacing: 10
    Button {
        width: 80
        height: 30
        text: {
            if (effectMode === "createMode")
                return "+";
            else
                return conditionName;
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
            else
                return effectName;
        }
        onClicked: {
            if (effectMode === "createMode") {
                effectMenu.popup();
            } else {
                selectEffect(componentId);
            }
        }

        CancelButton {
            anchors { horizontalCenter: effectButton.right; verticalCenter: effectButton.top }
            onClicked: deleteEffect(componentId)
            visible: effectMode !== "createMode"
        }

        EffectMenu {
            id: effectMenu
            onMenuClicked: createEffect(componentId, effectId)
            onAboutToShow: {
                effectMenu.x = effectButton.width;
                effectMenu.y = effectButton.height / 2 - effectMenu.height / 2;
            }
        }
    }
}
