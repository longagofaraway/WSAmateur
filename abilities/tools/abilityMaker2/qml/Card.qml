import QtQuick 2.12
import QtQuick.Controls 2.12

import 'Menus'
import 'BasicTypes'

Button {
    id: card

    signal createCardSpecifier(cardSpecifierType: string, value: string)

    property string displayName: 'Card'

    text: displayName
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
