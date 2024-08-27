import QtQuick 2.12
import QtQuick.Controls 2.12

Column {
    ComboBox {
        id: triggerTypeCombo
        width: 200
        model: ["Zone change", "On play", "On state change", "On attack", "On backup of this",
                "On trigger reveal", "Phase event", "End of this card's attack",
                "On standby trigger effect", "On being attacked", "On damage cancel",
                "On damage taken cancel", "On paying cost", "When 【ACT】 abillity used"]
        currentIndex: -1
        onCurrentIndexChanged: {
        }
    }
}
