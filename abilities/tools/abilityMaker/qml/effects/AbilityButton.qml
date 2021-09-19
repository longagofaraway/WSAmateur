import QtQuick 2.12
import QtQuick.Controls 2.12

Column {
    property int position

    signal editAbility(int pos)

    spacing: 5

    Button {
        text: "Ability " + position
        onClicked: editAbility(position)
    }
}
