import QtQuick 2.12
import QtQuick.Controls 2.12

Menu {
    id: targetMenu
    title: "Target"
    signal setTargetType(targetType: string)
    signal setTargetMode(targetMode: string)
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnReleaseOutside

    MenuItem {
        text: "This card"
        onTriggered: {
            console.log("menu log")
            targetMenu.setTargetType("ThisCard")
        }
    }
    MenuItem {
        text: "Chosen cards"
        onTriggered: targetMenu.setTargetType("ChosenCards")
    }
    MenuItem {
        text: "Revealed/Being looked at"
        onTriggered: targetMenu.setTargetType("MentionedCards")
    }
    MenuItem {
        text: "Rest of the cards"
        onTriggered: targetMenu.setTargetType("RestOfTheCards")
    }
    MenuItem {
        text: "Last moved cards"
        onTriggered: targetMenu.setTargetType("LastMovedCards")
    }
    MenuItem {
        text: "Battle opponent"
        onTriggered: targetMenu.setTargetType("BattleOpponent")
    }
    MenuItem {
        text: "Character in battle"
        onTriggered: targetMenu.setTargetType("CharInBattle")
    }
    MenuItem {
        text: "Attacking character"
        onTriggered: targetMenu.setTargetType("AttackingChar")
    }
    MenuItem {
        text: "Opposite this"
        onTriggered: targetMenu.setTargetType("OppositeThis")
    }
    MenuItem {
        text: "Mentioned in trigger"
        onTriggered: targetMenu.setTargetType("MentionedInTrigger")
    }

    Menu {
        cascade: true
        title: "Positional"
        MenuItem {
            text: "Any"
            onTriggered: targetMenu.setTargetMode("Any")
        }
        MenuItem {
            text: "All"
            onTriggered: setTargetMode("All")
        }
        MenuItem {
            text: "All other"
            onTriggered: setTargetMode("AllOther")
        }
        MenuItem {
            text: "In front of this"
            onTriggered: setTargetMode("InFrontOfThis")
        }
        MenuItem {
            text: "Front row"
            onTriggered: setTargetMode("FrontRow")
        }
        MenuItem {
            text: "Back row"
            onTriggered: setTargetMode("BackRow")
        }
        MenuItem {
            text: "Front row other"
            onTriggered: setTargetMode("FrontRowOther")
        }
        MenuItem {
            text: "Back row other"
            onTriggered: setTargetMode("BackRowOther")
        }
        MenuItem {
            text: "Middle position"
            onTriggered: setTargetMode("FrontRowMiddlePosition")
        }
        MenuItem {
            text: "Middle position other"
            onTriggered: setTargetMode("FrontRowMiddlePositionOther")
        }
    }
}
