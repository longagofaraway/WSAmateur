import QtQuick 2.12
import QtQuick.Controls 2.12

import "../basicTypes"

Rectangle {
    id: effectImpl

    signal editTarget()
    signal editPlace()
    signal typeChanged(int value)
    signal traitTypeChanged(int value)
    signal placeTypeChanged(int value)
    signal durationChanged(int value)

    width: root.width

    MouseArea {
        anchors.fill: parent
    }

    Text {
        id: label
        anchors.horizontalCenter: effectImpl.horizontalCenter
        text: "Add Marker"
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

        Column {
            Text { text: "Trait Modification Type" }
            TraitModificationType {
                id: modificationType
                Component.onCompleted: {
                    valueChanged.connect(typeChanged)
                }
            }
        }

        Column {
            Text { text: "Trait Type" }
            TraitType {
                id: traitType
                Component.onCompleted: {
                    valueChanged.connect(traitTypeChanged)
                }
            }
        }

        Column {
            id: durationColumn
            Text { text: "Duration" }
            ComboBox {
                id: duration
                model: ["0", "1", "2"]
                onCurrentIndexChanged: durationChanged(currentIndex);
            }
        }
    }

    function setType(value) {
        modificationType.setValue(value);
    }

    function setTraitType(value) {
        traitType.setValue(value);
    }

    function setPlaceType(value) {
        placeType.setValue(value);
    }

    function setDuration(value) {
        duration.currentIndex = value;
    }
}
