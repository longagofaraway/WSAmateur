import QtQuick 2.12
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.12

import wsamateur 1.0

import ".."


DecksListWindow {
    id: deckListWindow

    signal addDeck()
    signal chooseDeck(string deckName, string thubnail)

    property bool canChooseDeck: false
    property bool local: true

    TableView {
        id: tableView

        property real tableCellWidth: 400
        property real tableCellHeight: 300

        anchors.fill: parent

        clip: true
        interactive: deckListWindow.local
        ScrollBar.vertical: ScrollBar {}

        model: deckListWindow.decksModel

        delegate: Rectangle {
            id: cell
            implicitWidth: tableView.tableCellWidth
            implicitHeight: tableView.tableCellHeight
            color: "#00000000"

            Card {
                id: deckThumbnail
                anchors.centerIn: cell
                mSource: model.thumbnail

                width: mainWindow.width * 0.0677
                height: mainWindow.height * 0.1685

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
                        font.pointSize: 10
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            if (!model.downloadInProgress) {
                                deckListWindow.downloadImages(model.row, model.column);
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
                id: addDeckBlock

                anchors.fill: parent
                visible: model.lastElement
                z: 10
                Text {
                    anchors.centerIn: parent
                    text: "Add deck"
                    color: "white"
                    font.pointSize: 28
                }

                RadialGradient {
                    anchors.fill: parent
                    horizontalRadius: tableView.tableCellWidth / 1.5
                    verticalRadius: tableView.tableCellHeight / 1.5
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
                        deckListWindow.addDeck();
                    }
                }
            }

            Item {
                id: deckChooserBlock

                anchors.fill: parent
                visible: deckListWindow.canChooseDeck && !model.lastElement && !model.invalidElement
                z: -1

                RadialGradient {
                    anchors.fill: parent
                    horizontalRadius: tableView.tableCellWidth
                    verticalRadius: tableView.tableCellHeight
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: "#30FFFFFF" }
                        GradientStop { position: 0.5; color: "#00FFFFFF" }
                    }
                    visible: deckListWindow.canChooseDeck && chooseDeckArea.containsMouse
                }

                MouseArea {
                    id: chooseDeckArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        deckListWindow.chooseDeck(model.deckName, model.thumbnail);
                    }
                }
            }
        }
    }
}
