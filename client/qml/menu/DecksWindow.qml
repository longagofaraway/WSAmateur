import QtQuick 2.12
import QtGraphicalEffects 1.12
import QtQuick.Controls 2.15

import wsamateur 1.0
import ".."


DeckMenu {
    id: deckMenu

    property real tableCellWidth: 400
    property real tableCellHeight: 300
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

    TableView {
        id: tableView

        anchors {
            left: sideMenu.right
            leftMargin: 20
            top: parent.top
            topMargin: 50
        }

        width: deckMenu.width - sideMenu.width - 40
        height: deckMenu.height - 80

        clip: true

        ScrollBar.vertical: ScrollBar {}

        model: deckMenu.decksModel

        delegate: Rectangle {
            id: cell
            implicitWidth: tableCellWidth
            implicitHeight: tableCellHeight
            color: "#00000000"

            Card {
                id: deckThumbnail
                anchors.centerIn: cell
                mSource: model.thumbnail

                width: deckMenu.width * 0.0677
                height: deckMenu.height * 0.1685

                visible: !model.lastElement && !model.invalidElement

                Rectangle {
                    visible: !model.hasAllImages
                    anchors.centerIn: parent
                    width: parent.width * 0.92
                    height: 20
                    color: "white"
                    border.width: 1
                    border.color: "black"

                    Rectangle {
                        id: progressBar
                        visible: model.downloadInProgress
                        anchors {
                            left: parent.left
                            top: parent.top
                            bottom: parent.bottom
                        }
                        width: parent.width * model.percent / 100
                        color: "green"
                    }

                    Text {
                        anchors.centerIn: parent
                        text: {
                            if (model.downloadInProgress)
                                return model.percent.toString() + "%";
                            if (model.errorMessage)
                                return model.errorMessage;
                            return "download images";
                        }
                        font.pointSize: 11
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            if (!model.downloadInProgress) {
                                deckMenu.downloadImages(model.row, model.column);
                            }
                        }
                    }
                }
            }

            Text {
                anchors {
                    bottom: cell.bottom
                    bottomMargin: 15
                    left: cell.left
                    right: cell.right
                }
                text: model.deckName
                clip: true
                color: "white"
                font.pointSize: 18
                horizontalAlignment: Text.AlignHCenter
                visible: !model.lastElement && !model.invalidElement
            }

            Item {
                anchors.fill: parent
                visible: model.lastElement
                Text {
                    anchors.centerIn: parent
                    text: "Add deck"
                    color: "white"
                    font.pointSize: 28
                }

                RadialGradient {
                    anchors.fill: parent
                    horizontalRadius: tableCellWidth / 1.5
                    verticalRadius: tableCellHeight / 1.5
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
                    onClicked: {
                        urlInput.show();
                    }
                }
            }
        }
    }

    UrlInput {
        id: urlInput

        anchors.fill: parent
        onAdd: {
            if (!deckMenu.addDeck(url))
                urlInput.setError();
        }
        onCancel: deckMenu.cancelRequest()
    }

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
