import QtQuick 2.12
import QtQuick.Controls 2.12

Item {
    property int position

    signal editEffect(int pos)

    Button {
        text: "Add effect"
        onClicked: editEffect(position)
    }
}
