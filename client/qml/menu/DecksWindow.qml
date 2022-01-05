import QtQuick 2.12
import QtGraphicalEffects 1.12
import QtQuick.Controls 2.15

import wsamateur 1.0


Item {
    id: deckMenu

    anchors.fill: parent

    Image {
        id: backgroundImg
        anchors.fill: parent
        source: "qrc:///resources/background_menu_side"
        fillMode: Image.PreserveAspectCrop
    }

    ColorOverlay {
        id: colorOverlay
        anchors.fill: backgroundImg
        source: backgroundImg
        color: "#90000000"
    }

    SideMenu {
        id: sideMenu
        activeSlot: 2
        anchors {
            left: parent.left
            top: parent.top
            bottom: parent.bottom
        }
        width: parent.width * 0.09
    }

    DeckList {
        id: tableView

        anchors {
            left: sideMenu.right
            leftMargin: 20
            top: parent.top
            topMargin: 50
        }

        width: deckMenu.width - sideMenu.width - 40
        height: deckMenu.height - 80

        onAddDeck: urlInput.show()
        onDeckDownloadError: {
            urlInput.activateAddButton();
            urlInput.setError(reason);
        }
        onDeckDownloadSuccess: {
            urlInput.activateAddButton();
            urlInput.visible = false;
        }
        onUnsupportedCardMet: {
            onDeckDownloadError("Deck has unsupported cards");

            let comp = Qt.createComponent("UnsupportedCardsError.qml");
            var errorWindow = comp.createObject(deckMenu);
            errorWindow.cards = unsupportedCards;
            errorWindow.anchors.fill = deckMenu;
        }
    }

    UrlInput {
        id: urlInput

        anchors.fill: parent
        onAdd: {
            if (!tableView.addDeckFromEncoreDecks(url))
                urlInput.setError();
        }
        onCancel: tableView.cancelRequest()
    }
}
