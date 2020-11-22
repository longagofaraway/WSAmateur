import QtQuick 2.12

Rectangle {
    id: stagePlace

    property bool opponent: false
    property int row

    property var contentCard: undefined
    property CardInfoFrame cardInfo: null

    width: root.cardWidth
    height: root.cardHeight

    anchors.horizontalCenter: parent.horizontalCenter
    anchors.verticalCenter: parent.verticalCenter
    anchors.verticalCenterOffset: {
        var a = parent.height * 0.03;
        if (row == 2)
            a = a * 2 + height;
        a += height / 2;
        if (opponent)
            a *= -1;
        return a;
    }

    color: "#30FFFFFF"

    DropArea {
        id: dropStage
        width: parent.width
        height: row == 1 ? parent.height * 3/2 : parent.height / 2
        x: 0
        y: row == 1 ? 0 : parent.height / 2

        onEntered: {
            if (opponent)
                return;
            stagePlace.color = "#80FFFFFF";
            root.stageDropTarget = stagePlace;
        }

        onExited: {
            if (opponent)
                return;
            root.stageDropTarget = undefined
            stagePlace.color = "#30FFFFFF";
        }
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true

        onEntered: {
            if (contentCard === undefined)
                return;

            stagePlace.z = 100;
            let comp = Qt.createComponent("CardInfoFrame.qml");
            let incubator = comp.incubateObject(stagePlace, { visible: false }, Qt.Asynchronous);
            let createdCallback = function(status) {
                if (status === Component.Ready) {
                    if (!containsMouse) {
                        incubator.object.destroy();
                        return;
                    }
                    if (cardInfo !== null)
                        cardInfo.destroy();
                    cardInfo = incubator.object;
                    cardInfo.anchors.left = stagePlace.right;
                    cardInfo.visible = true;
                }
            }
            if (incubator.status !== Component.Ready) {
                incubator.onStatusChanged = createdCallback;
            } else {
                createdCallback(Component.Ready);
            }
        }

        onExited: {
            stagePlace.z = 0;
            if (cardInfo !== null) {
                cardInfo.destroy();
                cardInfo = null;
            }
        }
    }
}
