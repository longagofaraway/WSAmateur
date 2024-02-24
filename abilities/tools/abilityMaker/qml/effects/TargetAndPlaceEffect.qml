import QtQuick 2.12
import QtQuick.Controls 2.12

import "../basicTypes"

Rectangle {
    id: effectImpl

    signal editTarget()
    signal editPlace()
    signal placeTypeChanged(int value)

    width: root.width

    MouseArea {
        anchors.fill: parent
    }

    Text {
        id: label
        anchors.horizontalCenter: effectImpl.horizontalCenter
        text: "Choose Trait"
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
            PlaceType {
                id: placeType
                onValueChanged: {
                    if (value == 2) {
                        place.enabled = true;
                    } else {
                        place.enabled = false;
                    }
                    placeTypeChanged(value);
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
    }

    function setPlaceType(value) {
        placeType.setValue(value);
    }
}
