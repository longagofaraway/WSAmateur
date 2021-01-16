import QtQuick 2.0
import QtQuick.Controls 2.12

Item {
    property int triggerType

    property int onZoneChange: 1
    property int onPlay: 2
    property int onReverse: 3
    property int onAttack: 4
    property int onBattleOpponentReversed: 5
    property int onBackupOfThis: 6
    property int onTriggerReveal: 7
    property int onPhaseEvent: 8
    property int onEndOfThisCardsAttack: 9
    property int onOppCharPlacedByStandbyTriggerReveal: 10
    property int onEndOfThisTurn: 11

    ComboBox {
        model: ["Placed from hand to stage", "Zone change", "2"]
        onCurrentIndexChanged: {
            switch (currentIndex) {
            case 0:
                triggerType = onZoneChange;
                break;
            case 1:
                triggerType = onZoneChange;
                break;
            case 2:
                triggerType = onZoneChange;
                break;
            case 3:
                triggerType = onZoneChange;
                break;
            case 4:
                triggerType = onZoneChange;
                break;
            case 5:
                triggerType = onZoneChange;
                break;
            case 6:
                triggerType = onZoneChange;
                break;
            case 7:
                triggerType = onZoneChange;
                break;
            case 8:
                triggerType = onZoneChange;
                break;
            case 9:
                triggerType = onZoneChange;
                break;
            case 10:
                triggerType = onZoneChange;
                break;
            case 11:
                triggerType = onZoneChange;
                break;

            }
        }
    }
}
