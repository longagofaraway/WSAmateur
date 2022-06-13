import QtQuick 2.12
import QtQuick.Controls 2.12

Column {
    id: trigger

    signal editTriggers()

    enabled: false

    Text {
        text: "Triggers:"
    }

    Button {
        text: "Open editor"
        onClicked: {
            editTriggers();
        }
    }
}
