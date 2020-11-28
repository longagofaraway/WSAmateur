import QtQuick 2.12
import QtQuick.Window 2.12

ListView {
    id: clockView

    x: root.width * 0.08
    y: root.height * 0.85
    width: contentWidth
    height: root.cardHeight
    spacing: -root.cardWidth / 3
    transformOrigin: Item.TopLeft
    scale: 0.6
    orientation: ListView.Horizontal
    interactive: false

    model: ListModel {
        ListElement { img: "qrc:///resources/images/imc" }
        ListElement { img: "qrc:///resources/images/imc2" }
        ListElement { img: "qrc:///resources/images/imc3" }
        ListElement { img: "qrc:///resources/images/imc4" }
        ListElement { img: "qrc:///resources/images/imc" }
        ListElement { img: "qrc:///resources/images/imc2" }
        ListElement { img: "qrc:///resources/images/imc3" }
        ListElement { img: "qrc:///resources/images/imc4" }
        ListElement { img: "qrc:///resources/images/imc3" }
        ListElement { img: "qrc:///resources/images/imc4" }
    }

    delegate: Component {
        MouseArea {
            id: cardDelegate

            width: root.cardWidth
            height: root.cardHeight
            hoverEnabled: true

            onEntered: {
                cardImgDelegate.state = "hovered";
            }
            onExited: {
                cardImgDelegate.state = "";
            }

            Card {
                id: cardImgDelegate

                property CardInfoFrame cardTextFrame: null

                source: img
                anchors.centerIn: cardDelegate

                states: State {
                    name: "hovered"
                    PropertyChanges {
                        target: cardImgDelegate
                        scale: 1.1
                        z: 100
                    }
                    ParentChange {
                        target: cardImgDelegate
                        parent: clockView.contentItem
                    }
                    StateChangeScript {
                        name: "textFrame"
                        script: createTextFrame(cardImgDelegate);
                    }
                }

                transitions: Transition {
                    from: "hovered"
                    to: ""
                    ScriptAction { script: destroyTextFrame(cardImgDelegate); }
                }
            }
        }
    }

    function createTextFrame(frameParent) {
        let comp = Qt.createComponent("CardInfoFrame.qml");
        let incubator = comp.incubateObject(root, { visible: false }, Qt.Asynchronous);
        let createdCallback = function(status) {
            if (status === Component.Ready) {
                if (frameParent.state !== "hovered") {
                    incubator.object.destroy();
                    return;
                }
                if (frameParent.cardTextFrame !== null)
                    frameParent.cardTextFrame.destroy();
                var textFrame = incubator.object;

                let cardMappedPoint = root.mapFromItem(frameParent, frameParent.x, frameParent.y);
                let cardOffset = frameParent.x * clockView.scale;
                let cardWidthAndScaleOffset = frameParent.width * clockView.scale * (frameParent.scale + 1) / 2;
                textFrame.x = clockView.x + cardOffset + cardWidthAndScaleOffset;
                textFrame.y = cardMappedPoint.y;

                let cardHeight = frameParent.height * clockView.scale * frameParent.scale;
                if (textFrame.height > cardHeight)
                    textFrame.y -= textFrame.height - cardHeight;
                textFrame.visible = true;
                frameParent.cardTextFrame = textFrame;
            }
        }
        if (incubator.status !== Component.Ready) {
            incubator.onStatusChanged = createdCallback;
        } else {
            createdCallback(Component.Ready);
        }
    }

    function destroyTextFrame(frameParent) {
        if (frameParent.cardTextFrame !== null) {
            frameParent.cardTextFrame.destroy();
            frameParent.cardTextFrame = null;
        }
    }
}
