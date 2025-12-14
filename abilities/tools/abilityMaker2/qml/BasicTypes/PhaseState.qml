import QtQuick 2.12
import QtQuick.Controls 2.12

ComboBox {
    id: phaseState

    property string displayName: 'PhaseState'
    signal valueChanged(string newValue, string compId)

    Connections {
        target: parentComponent

        function onSetPhaseState(newValue, compId) {
            if (componentId !== compId)
                return;
            currentIndex = indexOfValue(newValue);
        }
    }

    Text {
        text: phaseState.displayName
        anchors.bottom: phaseState.top
    }

    textRole: "key"
    valueRole: "value"
    model: ListModel {
        ListElement { key: "Start"; value: "Start" }
        ListElement { key: "End"; value: "End" }
    }
    currentIndex: -1
    onCurrentValueChanged: valueChanged(currentValue, componentId);
}
