import QtQuick 2.12
import QtQuick.Controls 2.12

Menu {
    signal menuClicked(effectId: string)

    title: "Effect"

    MenuItem {
        text: "Attribute gain"
        onTriggered: menuClicked("AttributeGain")
    }
    MenuItem {
        text: "Choose card"
        onTriggered: menuClicked("ChooseCard")
    }
    MenuItem {
        text: "Move card"
        onTriggered: menuClicked("MoveCard")
    }
    MenuItem {
        text: "Look"
        onTriggered: menuClicked("Look")
    }
    MenuItem {
        text: "Pay cost"
        onTriggered: menuClicked("PayCost")
    }
}
