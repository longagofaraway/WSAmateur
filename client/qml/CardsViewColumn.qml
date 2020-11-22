import QtQuick 2.12
import QtQml.Models 2.12

ListView {
    id: thisListView
    height: maxHeight()
    width: contentWidth + leftMargin + rightMargin
    spacing: -root.cardHeight * 0.7
    clip: true
    contentWidth: root.cardWidth
    leftMargin: root.cardWidth * 0.1 * 0.5
    rightMargin: root.cardWidth * 0.1 * 0.5
    topMargin: root.cardHeight * 0.1 * 0.5
    bottomMargin: root.cardHeight * 0.1 * 0.5
    visible: count

    function maxHeight() {
        var point = root.contentItem.mapFromItem(thisListView, x, y);
        var allHeight = contentHeight + topMargin + bottomMargin;
        if (point.y + allHeight > root.height)
            return root.height - point.y;
        return allHeight;
    }

    property var num
    property var predicate

    model: DelegateModel {
        model: listModel

        groups: [
            DelegateModelGroup {
                includeByDefault: false
                name: "visibleGroup"
            }
        ]
        filterOnGroup: "visibleGroup"

        items.onChanged: {
            let cur = 0
            while (cur < items.count) {
                let item = items.get(cur);
                if (predicate(item.model))
                    item.groups = "visibleGroup";
                else
                    cur++;
            }
        }

        delegate: Component {
            MouseArea {
                id: cardDelegate

                //property int visualIndex: DelegateModel.itemsIndex
                //Binding { target: cardImgDelegate; property: "visualIndex"; value: visualIndex }

                width: root.cardWidth
                height: root.cardHeight
                hoverEnabled: true

                onEntered: {
                    cardImgDelegate.state = "hovered";
                }
                onExited: {
                    cardImgDelegate.state = "";
                }
                onPressed: {
                    cardImgDelegate.state = "";
                }

                Card {
                    id: cardImgDelegate
                    source: img
                    anchors.centerIn: cardDelegate

                    property CardInfoFrame cardTextFrame: null
                    //property int visualIndex: 0

                    states: State {
                        name: "hovered"
                        PropertyChanges {
                            target: cardImgDelegate
                            scale: 1.1
                            z: 100
                        }
                        PropertyChanges {
                            target: thisListView
                            z: 100
                        }
                        ParentChange {
                            target: cardImgDelegate
                            parent: thisListView
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
    }

    function createTextFrame(frameParent) {
        let comp = Qt.createComponent("CardInfoFrame.qml");
        let incubator = comp.incubateObject(root.contentItem, { visible: false }, Qt.Asynchronous);
        let createdCallback = function(status) {
            if (status === Component.Ready) {
                if (frameParent.state !== "hovered") {
                    incubator.object.destroy();
                    return;
                }
                if (frameParent.cardTextFrame !== null)
                    frameParent.cardTextFrame.destroy();
                let textFrame = incubator.object;

                let listViewMappedPoint = root.contentItem.mapFromItem(thisListView, thisListView.x, thisListView.y);
                let cardMappedPoint = root.contentItem.mapFromItem(frameParent, frameParent.x, frameParent.y);
                textFrame.x = cardMappedPoint.x - textFrame.width;
                let scaleOffset = root.cardHeight * 0.1 * 0.5;
                textFrame.y = listViewMappedPoint.y + frameParent.y - scaleOffset;

                if (textFrame.y + textFrame.height > root.height)
                    textFrame.y -= textFrame.y + textFrame.height - root.height;
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
