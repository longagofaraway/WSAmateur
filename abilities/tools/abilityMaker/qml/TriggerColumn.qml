import QtQuick 2.12
import QtQuick.Controls 2.12

Column {
    id: trigger

    signal editTrigger()

    enabled: false

    Text {
        text: "Trigger:"
    }

    Button {
        text: "Open editor"
        onClicked: {
            editTrigger();
        }
    }
}
