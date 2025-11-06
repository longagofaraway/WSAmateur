import QtQuick 2.12
import QtQuick.Controls 2.12

import 'Menus'
import 'BasicTypes'

Button {
    id: target

    property string displayName
    property string secondLine
    signal setTargetType(string value)
    signal setTargetMode(string value)
    signal createCardSpecifier(cardSpecifierType: string, value: string)
    signal numModifierChanged(string value)
    signal numValueChanged(string value)

    function setNumMod(newValue) {
        number.setNumMod(newValue);
    }
    function setValue(newValue) {
        number.setValue(newValue);
    }

    text: {
        if (secondLine.length === 0)
            return displayName
        return displayName + '\n' + secondLine
    }
    width: 150

    onClicked: {
        targetMenu.popup();
        cardMenu.popup();
    }

    function showNumber() {
        number.visible = true;
    }
    function hideNumber() {
        number.visible = false;
    }
    function setSecondLine(line) {
        target.secondLine = line;
    }

    Row {
        anchors.bottom: target.top

        Number {
            id: number
            scale: 0.7
            transformOrigin: Item.Bottom
            visible: false
            fontSize: 14

            onNumModifierChanged: target.numModifierChanged(value)
            onValueChanged: target.numValueChanged(value)
        }
    }

    TargetMenu {
        id: targetMenu
        //onClosed: cardMenu.close()
        onAboutToShow: {
            targetMenu.x = target.width;
            targetMenu.y = target.height / 2 - targetMenu.height / 2;
        }
        onSetTargetType: {
            cardMenu.close()
            target.setTargetType(targetType)
        }
        onSetTargetMode: {
            cardMenu.close()
            target.setTargetMode(targetMode)
        }
    }

    CardMenu {
        id: cardMenu
        width: 100
        //onClosed: if (!targetMenu.activeFocus) targetMenu.close()
        onAboutToShow: {
            cardMenu.x = -cardMenu.width;
            cardMenu.y = target.height / 2 - cardMenu.height / 2;
        }
        onCreateCardSpecifier: {
            targetMenu.close()
            target.createCardSpecifier(cardSpecifierType, value);
        }
    }
}
