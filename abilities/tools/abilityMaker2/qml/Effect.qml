import QtQuick 2.12
import QtQuick.Controls 2.12

Rectangle {
    id: effect

    signal effectTypeChanged(string value)

    function setValue(newValue) {
        effectTypeCombo.dontTrigger = true;
        effectTypeCombo.currentIndex = effectTypeCombo.indexOfValue(newValue);
    }

    Text {
        id: header
        anchors { top: effect.top; topMargin: 10 }
        anchors.horizontalCenter: effect.horizontalCenter
        font.pointSize: 12
        text: "Effect"
    }

    ComboBox {
        id: effectTypeCombo

        property bool dontTrigger: false
        anchors { top: header.bottom; topMargin: 10 }
        anchors.horizontalCenter: effect.horizontalCenter
        width: 200
        textRole: "key"
        valueRole: "value"
        model: ListModel {
            ListElement { key: "Move Card"; value: "MoveCard" }
            ListElement { key: "Draw Card"; value: "DrawCard" }
            ListElement { key: "Flip Over"; value: "FlipOver" }
            ListElement { key: "Swap Cards"; value: "SwapCards" }
            ListElement { key: "Add Marker"; value: "AddMarker" }
            ListElement { key: "Remove Marker"; value: "RemoveMarker" }
            ListElement { key: "Standby"; value: "Standby" }
            ListElement { key: "Move Wr to Deck"; value: "MoveWrToDeck" }
            ListElement { key: "Stock Swap"; value: "StockSwap" }
            ListElement { key: "Shuffle"; value: "Shuffle" }
            ListElement { key: "Choose Card"; value: "ChooseCard" }
            ListElement { key: "Look"; value: "Look" }
            ListElement { key: "Reveal Card"; value: "RevealCard" }
            ListElement { key: "Search Card"; value: "SearchCard" }
            ListElement { key: "Attribute Gain"; value: "AttributeGain" }
            ListElement { key: "Ability Gain"; value: "AbilityGain" }
            ListElement { key: "Early Play"; value: "EarlyPlay" }
            ListElement { key: "Cost Substitution"; value: "CostSubstitution" }
            ListElement { key: "Trigger Check Twice"; value: "TriggerCheckTwice" }
            ListElement { key: "Pay Cost"; value: "PayCost" }
            ListElement { key: "Perform Effect"; value: "PerformEffect" }
            ListElement { key: "Perform Replay"; value: "PerformReplay" }
            ListElement { key: "Delayed Ability"; value: "DelayedAbility" }
            ListElement { key: "Change State"; value: "ChangeState" }
            ListElement { key: "Deal Damage"; value: "DealDamage" }
            ListElement { key: "Shot Trigger Damage"; value: "ShotTriggerDamage" }
            ListElement { key: "Backup"; value: "Backup" }
            ListElement { key: "Bond"; value: "Bond" }
            ListElement { key: "Cannot Attack"; value: "CannotAttack" }
            ListElement { key: "Cannot Play"; value: "CannotPlay" }
            ListElement { key: "Cannot Use Backup or Event"; value: "CannotUseBackupOrEvent" }
            ListElement { key: "Char Auto Cannot Deal Damage"; value: "CharAutoCannotDealDamage" }
            ListElement { key: "Opponent Auto Cannot Deal Damage"; value: "OpponentAutoCannotDealDamage" }
            ListElement { key: "Cannot Become Reversed"; value: "CannotBecomeReversed" }
            ListElement { key: "Cannot Move"; value: "CannotMove" }
            ListElement { key: "CannotStand"; value: "CannotStand" }
            ListElement { key: "Cannot Be Chosen"; value: "CannotBeChosen" }
            ListElement { key: "Side Attack Without Penalty"; value: "SideAttackWithoutPenalty" }
            ListElement { key: "Put On Stage Rested"; value: "PutOnStageRested" }
            ListElement { key: "Trigger Icon Gain"; value: "TriggerIconGain" }
            ListElement { key: "Can Play Without Color Requirement"; value: "CanPlayWithoutColorRequirement" }
            ListElement { key: "Replay"; value: "Replay" }
            ListElement { key: "Skip Phase"; value: "SkipPhase" }
            ListElement { key: "Choose Trait"; value: "ChooseTrait" }
            ListElement { key: "Trait Modification"; value: "TraitModification" }
        }
        currentIndex: -1
        onCurrentValueChanged: {
            if (dontTrigger) {
                dontTrigger = false;
                return;
            }

            effectTypeChanged(currentValue);
        }
    }
}
