import QtQuick 2.12
import QtQuick.Controls 2.12

import "../basicTypes"

Rectangle {
    id: multiplierImpl

    signal editTarget()
    signal placeTypeChanged(int value)
    signal editPlace()
    signal editMarkerBearer()

    width: root.width

    MouseArea {
        anchors.fill: parent
    }

    Text {
        id: label
        anchors.horizontalCenter: multiplierImpl.horizontalCenter
        text: "For Each"
        font.pointSize: 12
    }

    Row {
        spacing: 5
        anchors{ top: label.bottom; topMargin: 10 }
        anchors.horizontalCenter: multiplierImpl.horizontalCenter

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
                        markerBearer.enabled = false;
                    } else if (value == 4) {
                        place.enabled = false;
                        markerBearer.enabled = true;
                    } else {
                        place.enabled = false;
                        markerBearer.enabled = false;
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

        Column {
            id: markerBearer
            Text { text: "Marker Bearer" }
            Button {
                text: "Open editor"
                onClicked: {
                    editMarkerBearer();
                }
            }
        }
    }

    function setPlaceType(value) {
        placeType.setValue(value);
    }
}
