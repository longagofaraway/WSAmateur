import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Styles 1.4

import 'Menus'
import abilityMaker 1.0

Window {
    id: window
    visible: true
    width: 780
    height: 580
    title: qsTr("Ability Maker")

    AbilityMaker {
        id: root
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 10

        Connections {
            target: rootAbility
            function onComponentChanged(ability) {
                abilityText.text = root.translate(ability);
            }
        }

        Ability {
            id: rootAbility
            anchors { top: root.top; left: root.left; right: root.right; bottom: abilityText.top }
        }

        AbilityText {
            id: abilityText
            anchors { left: root.left; right: root.right; bottom: root.bottom }
        }
    }
}
