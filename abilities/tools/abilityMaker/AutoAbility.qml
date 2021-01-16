import QtQuick 2.0
import QtQuick.Controls 2.12

Item {
    id: autoAbility

    property int activationTimes: 0
    property var keywords: []
    property string trigger

    width: row.width
    x: root.width / 2 - width / 2;
    Component.onCompleted: {
        y = root.childrenRect.height;
    }

    Row {
        id: row
        spacing: 3

        Column {
            Text {
                text: "Activates up to:"
            }

            ComboBox {
                model: ["always", "1", "2"]
                onCurrentIndexChanged: activationTimes = currentIndex;
            }
        }

        Column {
            id: keywordCol

            property var keywordBoxes: []

            Text {
                text: "Keywords:"
            }

            Button {
                id: keywordsBtn
                text: "Add keyword"
                onClicked: {
                    if (keywordCol.keywordBoxes.length == 0)
                        endKeywords.visible = true;
                    let component = Qt.createComponent("qrc:/Keywords.qml");
                    let keyword = component.createObject(keywordCol);
                    keyword.index = autoAbility.keywords.length;
                    autoAbility.keywords.push(0);
                    keyword.selected.connect(keywordCol.addKeyword);
                    keywordCol.keywordBoxes.push(keyword);
                }
            }
            Button {
                id: endKeywords
                text: "Finish with keywords"
                visible: false
                onClicked: {
                    keywordsBtn.text = "";
                    for (let i = 0; i < keywordCol.keywordBoxes.length; i++) {
                        keywordCol.keywordBoxes[i].visible = false;
                        keywordsBtn.text += keywordCol.keywordBoxes[i].currentText + " ";
                    }
                    keywordsBtn.enabled = false;
                    endKeywords.visible = false;
                }
            }

            function addKeyword(index, keyword) {
                autoAbility.keywords[index] = keyword;
            }
        }

        Column {
            Text {
                text: "Keywords:"
            }
            Button {
                text: "Trigger:"
                onClicked: {
                    autoAbility.trigger = root.createComponent("Trigger");
                }
            }
        }
    }

}
