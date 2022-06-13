import QtQuick 2.12
import QtQuick.Controls 2.12

Column {
    property int position

    signal editTrigger(int pos)

    spacing: 5

    Button {
        text: "Trigger " + position
        onClicked: editTrigger(position)
    }
}
