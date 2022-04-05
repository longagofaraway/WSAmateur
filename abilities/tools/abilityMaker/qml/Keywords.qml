import QtQuick 2.12
import QtQuick.Controls 2.12

Column {
    id: keywords

    signal selected(var keywords)

    enabled: false

    property var keywordsList: []

    Text {
        text: "Keywords"
    }

    Row {
    Label {
        id: keywordsLabel
        width: rootCombo.width
        height: rootCombo.height
        verticalAlignment: Text.AlignVCenter
        text: ""
        background: Rectangle {
            color: "#eeeeee"
        }
    }
    Button {
        width: resetKeywords.contentWidth
        Text {
            id: resetKeywords
            text: "⟳"
            color: "red"
            font.pointSize: 20
        }

        onClicked: {
            keywords.keywordsList = [];
            ability.setKeywords(keywords.keywordsList);
            keywordsLabel.text = "";
        }
    }
    }

    Text {
        text: "Add keyword"
    }

    ComboBox {
        model: ["Encore", "CxCombo", "Brainstorm", "Backup", "Experience", "Resonance", "Bond", "Replay", "Alarm", "Change", "Assist"]
        currentIndex: -1
        onCurrentIndexChanged: {
            if (currentIndex === -1)
                return;

            keywords.keywordsList.push(model[currentIndex]);
            keywords.selected(keywords.keywordsList);
            if (keywordsLabel.text.length !== 0)
                keywordsLabel.text += ", ";
            keywordsLabel.text += model[currentIndex];
            currentIndex = -1;
        }
    }

    function setKeywords(keywordsString) {
        keywordsLabel.text = keywordsString;
    }
}
