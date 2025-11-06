import QtQuick 2.12
import QtQuick.Controls 2.12

CheckBox {
    property string displayName
    signal valueChanged(bool newValue, string compId)

    function setValue(newValue) {
        checked = newValue;
    }

    text: displayName
    checked: false
    onCheckedChanged: {
        valueChanged(checked, componentId);
    }
}
