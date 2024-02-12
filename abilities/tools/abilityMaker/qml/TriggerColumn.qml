import QtQuick 2.12
import QtQuick.Controls 2.12

import 'utilities'

Column {
    id: trigger

    signal editTriggers()
    signal setTrigger(triggerName: string)

    enabled: false

    Text {
        text: "Triggers:"
    }

    AdvancedButton {
        text: "Open editor"
        onClicked: {
            editTriggers();
        }
        onRightClicked: {
            contextMenu.popup()
        }

        Menu {
            id: contextMenu
            MenuItem {
                text: "Placed from hand to stage"
                onTriggered: setTrigger(text)
            }
            MenuItem {
                text: "When this card attacks"
                onTriggered: setTrigger(text)
            }
            MenuItem {
                text: "When this becomes reversed"
                onTriggered: setTrigger(text)
            }
        }
    }
}
