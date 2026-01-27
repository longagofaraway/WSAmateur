import QtQuick 2.12
import QtQuick.Controls 2.12

Menu {
    signal menuClicked(conditionId: string)

    title: "Condition"

    MenuItem {
        text: "No condition"
        onTriggered: menuClicked("NoCondition")
    }
    MenuItem {
        text: "Have cards"
        onTriggered: menuClicked("HaveCards")
    }
    MenuItem {
        text: "Card Is"
        onTriggered: menuClicked("IsCard")
    }
    MenuItem {
        text: "In Battle With This"
        onTriggered: menuClicked("InBattleWithThis")
    }
    MenuItem {
        text: "Sum of levels"
        onTriggered: menuClicked("SumOfLevels")
    }
    MenuItem {
        text: "Cards location"
        onTriggered: menuClicked("CardsLocation")
    }
    MenuItem {
        text: "Revealed card is"
        onTriggered: menuClicked("RevealedCard")
    }
}
