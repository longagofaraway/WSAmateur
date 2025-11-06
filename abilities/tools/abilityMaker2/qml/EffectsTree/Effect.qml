import QtQuick 2.12
import QtQuick.Controls 2.12

import '../Menus'

Row {
    signal createEffect(string compId, string effect)
    property string componentId
    property string effectMode: "createMode"
    property string effectName
    property string conditionName

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
        text: {
            if (effectMode === "createMode")
                return "+";
            else
                return effectName;
        }
        onClicked: {
            if (effectMode === "createMode") {
                effectMenu.popup();
            }
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
