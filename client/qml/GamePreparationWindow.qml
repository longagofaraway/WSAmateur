import QtQuick 2.12
import QtGraphicalEffects 1.12
import QtQuick.Controls 2.15

import wsamateur 1.0

import "menu"

GamePreparation {
    id: gamePreparation

    property var deckList: null
    property bool deckSet: false
    property bool playerReady: false

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

    Rectangle {
        id: delimiter
        anchors { horizontalCenter: parent.horizontalCenter }
        width: 5; height: parent.height
        gradient: Gradient {
            orientation: Gradient.Vertical
            GradientStop { position: 0.0; color: "#00F0F0F0" }
            GradientStop { position: 0.1; color: "#00F0F0F0" }
            GradientStop { position: 0.34; color: "#F0F0F0" }
            GradientStop { position: 0.66; color: "#F0F0F0" }
            GradientStop { position: 0.9; color: "#00F0F0F0" }
            GradientStop { position: 1.0; color: "#00F0F0F0" }
        }
    }

    Item {
        id: deckChooser

        x: delimiter.x / 2 - width / 2
        y: 50
        width: gamePreparation.width / 4
        height: gamePreparation.height / 8

        Text {
            id: chooseText
            anchors.centerIn: parent
            text: "Choose deck"
            color: "white"
            font.pointSize: 28
        }

        RadialGradient {
            anchors.fill: parent
            horizontalRadius: deckChooser.width / 1.5
            verticalRadius: deckChooser.height / 1.5
            gradient: Gradient {
                GradientStop { position: 0.0; color: "#30FFFFFF" }
                GradientStop { position: 0.5; color: "#00FFFFFF" }
            }
            visible: addDeckArea.containsMouse
        }

        MouseArea {
            id: addDeckArea
            anchors.fill: parent
            hoverEnabled: true
            onClicked: createDeckListWindow()
        }
    }

    Item {
        id: deckRepresentation

        anchors.top: deckChooser.bottom
        x: delimiter.x / 2 - width / 2
        width: gamePreparation.width / 4
        height: gamePreparation.height / 4
        visible: false

        Card {
            id: deckThumbnail
            anchors.centerIn: parent
            mSource: "cardback"

            width: mainWindow.width * 0.0677
            height: mainWindow.height * 0.1685
        }

        Text {
            id: deckText
            anchors {
                top: deckThumbnail.bottom
                topMargin: 15
                horizontalCenter: deckThumbnail.horizontalCenter
            }
            clip: true
            color: "white"
            font.pointSize: 18
            horizontalAlignment: Text.AlignHCenter
        }
    }

    MenuButton {
        id: readyButton

        anchors.top: deckRepresentation.bottom
        anchors.topMargin: 40
        x: delimiter.x / 2 - width / 2
        visible: deckRepresentation.visible

        text: "Ready"
        onPressed: {
            if (!gamePreparation.deckSet || gamePreparation.playerReady)
                return;
            addDeckArea.enabled = false;
            chooseText.text = "You have chosen the deck";
            gamePreparation.sendDeck(deckText.text);
            gamePreparation.setReady();
            gamePreparation.playerReady = true;
            readyButton.visible = false;
        }
    }

    Item {
        id: opponentsHalf

        anchors {
            left: delimiter.right
            right: gamePreparation.right
            top: gamePreparation.top
            bottom: gamePreparation.bottom
        }

        Text {
            id: opponentsDeckHeader

            anchors.horizontalCenter: parent.horizontalCenter
            y: 50
            height: gamePreparation.height / 8
            text: "Opponent is choosing a deck..."
            color: "white"
            font.pointSize: 24
            verticalAlignment: Text.AlignVCenter
        }

        DeckList {
            id: opponentdDeckRepresentation

            anchors {
                top: opponentsDeckHeader.bottom
                horizontalCenter: parent.horizontalCenter
            }
            width: 400
            height: 300
            local: false

            onDeckImagesDownloaded: {
                loadingLabel.visible = false;
                gamePreparation.oppDeckImagesDownloaded();
            }
        }

        Text {
            id: loadingLabel
            anchors {
                top: opponentdDeckRepresentation.bottom
                topMargin: 40
                horizontalCenter: parent.horizontalCenter
            }
            text: "Images dowloading..."
            color: "white"
            font.pointSize: 20
            visible: false

            SequentialAnimation on opacity {
                loops: Animation.Infinite
                PropertyAnimation { to: 0; duration: 1000 }
                PropertyAnimation { to: 1; duration: 1000 }
            }
        }
    }

    function deckChosen(deckName, thumbnail) {
        deckThumbnail.mSource = thumbnail;
        deckText.text = deckName;
        deckRepresentation.visible = true;
        backgroundImg.z = 0;
        colorOverlay.z = 0;
        deckList.destroy();
        deckList = null;
        gamePreparation.deckSet = true;
    }

    function createDeckListWindow() {
        let comp = Qt.createComponent("menu/DeckList.qml");
        deckList = comp.createObject(gamePreparation);
        deckList.anchors.fill = gamePreparation;
        deckList.anchors.leftMargin = 20;
        deckList.anchors.rightMargin = 20;
        deckList.anchors.topMargin = 50;
        deckList.canChooseDeck = true;
        backgroundImg.z = 10;
        colorOverlay.z = 11;
        deckList.z = 12;

        deckList.chooseDeck.connect(gamePreparation.deckChosen);
    }

    onSigSetOpponentDeck: {
        loadingLabel.visible = true;
        if (!opponentdDeckRepresentation.addDeckToModel(xmlDeck))
            return;
        opponentdDeckRepresentation.visible = true;
        opponentsDeckHeader.text = "Opponent has chosen his deck";
    }
}
