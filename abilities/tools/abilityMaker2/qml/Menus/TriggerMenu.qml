import QtQuick 2.12
import QtQuick.Controls 2.12

Menu {
    title: "Triggers"
    signal createTrigger(triggerId: string)
    Menu {
        cascade: true
        title: "Zone change"
        MenuItem {
            text: "Hand -> Stage"
            onTriggered: createTrigger("ZoneChangeHandStage")
        }
        MenuItem {
            text: "Stage -> Waiting room"
            onTriggered: createTrigger("ZoneChangeStageWr")
        }
        MenuItem {
            text: "Climax is placed"
            onTriggered: createTrigger("ZoneChangeCxPlaced")
        }
    }
    MenuItem {
        text: "On attack"
        onTriggered: createTrigger("OnAttack")
    }
    Menu {
        cascade: true
        title: "On reverse"
        MenuItem {
            text: "This card"
            onTriggered: createTrigger("OnReverseThis")
        }
        MenuItem {
            text: "Battle opponent"
            onTriggered: createTrigger("OnReverseOpp")
        }
    }
    Menu {
        cascade: true
        title: "Phase event"
        MenuItem {
            text: "Your climax phase"
            onTriggered: createTrigger("YourCxPhase")
        }
        MenuItem {
            text: "Your encore step"
            onTriggered: createTrigger("YourEncore")
        }
        MenuItem {
            text: "Opponent attack phase"
            onTriggered: createTrigger("OppAttackPhase")
        }
    }
    MenuItem {
        text: "End of this card's attack"
        onTriggered: createTrigger("EndOfCardsAttack")
    }
    MenuItem {
        text: "Custom trigger"
        onTriggered: createTrigger("CustomTrigger")
    }
}
