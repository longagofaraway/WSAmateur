import QtQuick 2.12
import QtQuick.Controls 2.12

import "../basicTypes"

Rectangle {
    id: effectImpl

    signal editTarget()
    signal editFrom()
    signal editTo()
    signal editTo2()
    signal addDestination()
    signal orderChanged(int value)
    signal executorChanged(int value)

    width: root.width

    MouseArea {
        anchors.fill: parent
    }

    Text {
        id: label
        anchors.horizontalCenter: effectImpl.horizontalCenter
        text: "Move Card"
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
            id: from
            Text { text: "From" }
            Button {
                text: "Open editor"
                onClicked: {
                    editFrom();
                }
            }
        }

        Column {
            id: to
            Text { text: "To" }
            Button {
                text: "Open editor"
                onClicked: {
                    editTo();
                }
            }
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "+"
            }
            Button {
                property bool created: false
                text: "Add Destination"
                onClicked: {
                    if (!created) {
                        addDestination();
                        text = "Open Editor";
                        created = true;
                    } else {
                        editTo2();
                    }
                }
            }
        }

        Column {
            Text { text: "Order" }
            ComboBox {
                id: order
                model: ["Not Specified", "Any", "Same"]
                onCurrentIndexChanged: {
                    orderChanged(currentIndex);
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

    function setOrder(value) {
        order.currentIndex = value;
    }

    function setExecutor(value) {
        executor.setValue(value);
    }
}
