import QtQuick 2.12
import QtQuick.Controls 2.12

import "../basicTypes"
import ".."

Rectangle {
    id: effectImpl

    signal componentReady()
    signal cancel()

    signal editTarget()
    signal editTarget2()
    signal placeTypeChanged(int value)
    signal placeType2Changed(int value)
    signal editPlace()
    signal editPlace2()
    signal executorChanged(int value)
    signal createSecondTarget()

    signal incomingPlaceType(int value)
    onIncomingPlaceType: {
        placeType.currentIndex = value - 1;
    }
    signal incomingExecutor(int value)
    onIncomingExecutor: {
        executor.setValue(value);
    }

    width: root.width
    height: root.height - root.pathHeight - root.textAreaHeight - 100

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
            Row {
                spacing: 5
                //anchors{ top: label.bottom; topMargin: 10 }
                anchors.horizontalCenter: parent.horizontalCenter

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
            }
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "+ (or)"
            }
            Row {
                spacing: 5
                anchors.horizontalCenter: parent.horizontalCenter

                Button {
                    id: addTargetButton
                    property bool created: false
                    text: "Or choose in another place"
                    onClicked: {
                        if (!created) {
                            addSecondTarget();
                            createSecondTarget();
                        }
                    }
                }

                Button {
                    id: editor2
                    visible: false
                    text: "Open editor"
                    onClicked: {
                        editTarget2();
                    }
                }

                ComboBox {
                    id: placeType2
                    visible: false
                    model: ["Revealed/Looked at", "Specific place"]
                    currentIndex: 1
                    onCurrentIndexChanged: {
                        if (currentIndex == 1)
                            place2.enabled = true;
                        else
                            place2.enabled = false;
                        placeType2Changed(currentIndex + 1);
                    }
                }

                Button {
                    id: place2
                    visible: false
                    text: "Open editor"
                    onClicked: {
                        editPlace2();
                    }
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

    ConfirmButton {
        id: confirmButton
        visible: false
        onClicked: componentReady()
    }

    CancelButton {
        id: cancelButton
        visible: false
        onClicked: cancel()
    }

    function setActualY() {
        y = -mapToItem(root, 0, 0).y + root.pathHeight;
    }

    function enableButtons() {
        confirmButton.visible = true;
        cancelButton.visible = true;
    }

    function setPlaceType(value) {
        placeType.currentIndex = value - 1;
    }

    function setExecutor(value) {
        executor.setValue(value);
    }

    function addSecondTarget() {
        addTargetButton.created = true;
        addTargetButton.visible = false;
        editor2.visible = true;
        placeType2.visible = true;
        place2.visible = true;
    }

    function setSecondTarget(value) {
        placeType2.currentIndex = value - 1;
        addSecondTarget();
    }
}
