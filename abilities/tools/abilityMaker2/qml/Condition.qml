import QtQuick 2.12
import QtQuick.Controls 2.12

Rectangle {
    id: condition

    signal conditionTypeChanged(string value)

    function setValue(newValue) {
        conditionTypeCombo.dontTrigger = true;
        conditionTypeCombo.currentIndex = conditionTypeCombo.indexOfValue(newValue);
    }

    Text {
        id: header
        anchors { top: condition.top; topMargin: 10 }
        anchors.horizontalCenter: condition.horizontalCenter
        font.pointSize: 12
        text: "Condition"
    }

    ComboBox {
        id: conditionTypeCombo

        property bool dontTrigger: false
        anchors { top: header.bottom; topMargin: 10 }
        anchors.horizontalCenter: condition.horizontalCenter
        width: 200
        textRole: "key"
        valueRole: "value"
        model: ListModel {
            ListElement { key: "No Condition"; value: "NoCondition" }
            ListElement { key: "Is Card"; value: "IsCard" }
            ListElement { key: "Have Cards"; value: "HaveCards" }
            ListElement { key: "And"; value: "And" }
            ListElement { key: "Or"; value: "Or" }
            ListElement { key: "In Battle With This"; value: "InBattleWithThis" }
            ListElement { key: "Sum Of Levels"; value: "SumOfLevels" }
            ListElement { key: "Card's Location"; value: "CardsLocation" }
            ListElement { key: "During Turn"; value: "DuringTurn" }
            ListElement { key: "Check Milled Cards"; value: "CheckMilledCards" }
            ListElement { key: "Revealed Card"; value: "RevealedCard" }
            ListElement { key: "Player's Level"; value: "PlayersLevel" }
            ListElement { key: "During Card's First Turn"; value: "DuringCardsFirstTurn" }
            ListElement { key: "Card Moved"; value: "CardMoved" }
            ListElement { key: "Performed In Full"; value: "PerformedInFull" }
            ListElement { key: "Has Markers"; value: "HasMarkers" }
        }
        currentIndex: -1
        onCurrentValueChanged: {
            if (dontTrigger) {
                dontTrigger = false;
                return;
            }

            conditionTypeChanged(currentValue);
        }
    }
}
