import QtQuick 2.9
import QtQuick.Controls 2.2

ComboBox {
        id: multiComboBox

        displayText: "Select Keywords"

        // ComboBox closes the popup when its items (anything AbstractButton derivative) are
        //  activated. Wrapping the delegate into a plain Item prevents that.
        delegate: Item {
            width: parent.width
            height: checkDelegate.height

            function toggle() { checkDelegate.toggle() }

            CheckDelegate {
                id: checkDelegate
                anchors.fill: parent
                text: model.name
                highlighted: multiComboBox.highlightedIndex == index
                checked: model.selected
                onCheckedChanged: {
                    model.selected = checked;
                    let newText = "";
                    let count = 0
                    for (let i = 0; i < multiComboBox.model.count; i++) {
                        let object = multiComboBox.model.get(i);
                        if (object.selected) {
                            if (count > 0) {
                                newText += ", ";
                            }
                            count++;
                            newText += object.name;
                        }
                    }
                    if (newText.length > 0)
                        multiComboBox.displayText = newText;
                    else
                        multiComboBox.displayText = "Select Keywords";
                }
            }
        }

        // override space key handling to toggle items when the popup is visible
        Keys.onSpacePressed: {
            if (multiComboBox.popup.visible) {
                var currentItem = multiComboBox.popup.contentItem.currentItem
                if (currentItem) {
                    currentItem.toggle()
                    event.accepted = true
                }
            }
        }

        Keys.onReleased: {
            if (multiComboBox.popup.visible)
                event.accepted = (event.key === Qt.Key_Space)
        }
    }
