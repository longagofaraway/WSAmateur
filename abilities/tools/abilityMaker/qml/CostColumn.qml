import QtQuick 2.12
import QtQuick.Controls 2.12

import 'utilities'

Column {
    id: cost

    signal editCost()
    signal addCost(costName: string)

    enabled: false

    Text {
        text: "Cost:"
    }

    AdvancedButton {
        text: "Open editor"
        onClicked: {
            editCost();
        }
        onRightClicked: {
            contextMenu.popup()
        }

        Menu {
            id: contextMenu
            MenuItem {
                text: "Add stock cost"
                onTriggered: addCost(text)
            }
            MenuItem {
                text: "Add discard a card"
                onTriggered: addCost(text)
            }
            MenuItem {
                text: "Add rest this"
                onTriggered: addCost(text)
            }
        }
    }
}
