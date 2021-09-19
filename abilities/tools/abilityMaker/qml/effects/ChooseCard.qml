import QtQuick 2.12
import QtQuick.Controls 2.12

import "../basicTypes"

Rectangle {
    id: effectImpl

    signal editTarget()
    signal placeTypeChanged(int value)
    signal editPlace()
    signal executorChanged(int value)

    signal incomingPlaceType(int value)
    onIncomingPlaceType: {
        placeType.currentIndex = value - 1;
    }
    signal incomingExecutor(int value)
    onIncomingExecutor: {
        executor.setValue(value);
    }

    width: root.width

    MouseArea {
        anchors.fill: parent
    }

    Text {
        id: label
        anchors.horizontalCenter: effectImpl.horizontalCenter
        text: "Choose Card"
        font.pointSize: 12
    }

    Row {
        spacing: 5
        anchors{ top: label.bottom; topMargin: 10 }
        anchors.horizontalCenter: effectImpl.horizontalCenter

        Column {
            Text {
                text: "Target"
            }
            Button {
                text: "Open editor"
                onClicked: {
                    editTarget();
                }
            }
        }

        Column {
            Text { text: "Place type" }
            ComboBox {
                id: placeType
                model: ["Revealed/Looked at", "Specific place"]
                onCurrentIndexChanged: {
                    if (currentIndex == 1)
                        place.enabled = true;
                    else
                        place.enabled = false;
                    placeTypeChanged(currentIndex + 1);
                }
            }
        }

        Column {
            id: place
            Text { text: "Place" }
            Button {
                text: "Open editor"
                onClicked: {
                    editPlace();
                }
            }
        }

        Column {
            Text { text: "Executor" }
            Player {
                id: executor
                Component.onCompleted: {
                    valueChanged.connect(executorChanged)
                }
            }
        }
    }

    function setPlaceType(value) {
        placeType.currentIndex = value - 1;
    }

    function setExecutor(value) {
        executor.setValue(value);
    }
}
