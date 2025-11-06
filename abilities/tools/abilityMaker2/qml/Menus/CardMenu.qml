import QtQuick 2.12
import QtQuick.Controls 2.12

Menu {
    id: cardMenu
    title: "Card"
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnReleaseOutside

    signal createCardSpecifier(cardSpecifierType: string, value: string)

    Menu {
        cascade: true
        title: "Card type"
        MenuItem {
            text: "Character"
            onTriggered: cardMenu.createCardSpecifier("CardType", "Char")
        }
        MenuItem {
            text: "Climax"
            onTriggered: cardMenu.createCardSpecifier("CardType", "Climax")
        }
    }
    Menu {
        cascade: true
        title: "Owner"
        MenuItem {
            text: "Player"
            onTriggered: cardMenu.createCardSpecifier("Owner", "Player")
        }
        MenuItem {
            text: "Opponent"
            onTriggered: cardMenu.createCardSpecifier("Owner", "Opponent")
        }
        MenuItem {
            text: "Both"
            onTriggered: cardMenu.createCardSpecifier("Owner", "Both")
        }
    }
    MenuItem {
        text: "Trait"
        onTriggered: cardMenu.createCardSpecifier("Trait", "")
    }
    MenuItem {
        text: "Exact name"
        onTriggered: cardMenu.createCardSpecifier("ExactName", "")
    }
    MenuItem {
        text: "Name contains"
        onTriggered: cardMenu.createCardSpecifier("NameContains", "")
    }
    MenuItem {
        text: "Level"
        onTriggered: cardMenu.createCardSpecifier("Level", "")
    }
    Menu {
        cascade: true
        title: "State"
        MenuItem {
            text: "Standing"
            onTriggered: cardMenu.createCardSpecifier("State", "Standing")
        }
        MenuItem {
            text: "Rested"
            onTriggered: cardMenu.createCardSpecifier("State", "Rested")
        }
        MenuItem {
            text: "Reversed"
            onTriggered: cardMenu.createCardSpecifier("State", "Reversed")
        }
    }
}
