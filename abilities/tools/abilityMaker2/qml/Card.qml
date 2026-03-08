import QtQuick 2.12
import QtQuick.Controls 2.12

import 'Menus'
import 'BasicTypes'


Rectangle {
    id: card

    signal createCardSpecifier(cardSpecifierType: string, value: string)

    property string displayName: 'Card'

    width: 150
    height: childrenRect.height

Button {
    id: cardButton

    text: card.displayName
    width: 150

    onClicked: cardMenu.popup()

    CardMenu {
        id: cardMenu
        width: 100
        /*onAboutToShow: {
            cardMenu.x = -cardMenu.width;
            cardMenu.y = target.height / 2 - cardMenu.height / 2;
        }*/
        onCreateCardSpecifier: {
            card.createCardSpecifier(cardSpecifierType, value);
        }
    }
}
}
