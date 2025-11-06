import QtQuick 2.12
import QtQuick.Controls 2.12

Menu {
    title: "Cost"

    MenuItem {
        text: "Stock"
    }
    Menu {
        cascade: true
        title: "Discard"
        MenuItem {
            text: "Card"
        }
        MenuItem {
            text: "Character"
        }
        MenuItem {
            text: "Climax"
        }
    }
    MenuItem {
        text: "Tap this"
    }
    Menu {
        cascade: true
        title: "Move card"
        MenuItem {
            text: "This -> Waiting Room"
        }
        MenuItem {
            text: "Top deck -> Clock"
        }
    }

}
