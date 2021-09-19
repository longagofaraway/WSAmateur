import QtQuick 2.12
import QtQuick.Controls 2.12

import ".."

Rectangle {
    id: triggerImpl

    signal editCard()
    signal clearCard()

    width: root.width

    MouseArea {
        anchors.fill: parent
    }

    Text {
        id: label
        anchors.horizontalCenter: triggerImpl.horizontalCenter
        text: "On Battle Opponent Reversed"
        font.pointSize: 12
    }

    CardColumn {
        id: card
        anchors{ top: label.bottom; topMargin: 10 }
        anchors.horizontalCenter: triggerImpl.horizontalCenter
        Component.onCompleted: {
            card.editCard.connect(triggerImpl.editCard);
            card.clearCard.connect(triggerImpl.clearCard);
        }
    }
}
