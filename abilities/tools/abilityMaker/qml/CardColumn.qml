import QtQuick 2.12
import QtQuick.Controls 2.12

Column {
    signal editCard()
    signal clearCard()

    Text {
        text: "Card filters:"
    }

    Row {
    Button {
        id: openBtn
        text: "Open editor"
        onClicked: {
            editCard();
        }
    }
    Button {
        width: resetCard.contentWidth
        Text {
            id: resetCard
            text: "⟳"
            color: "red"
            font.pointSize: 20
        }

        onClicked: clearCard()
    }
    }
}
