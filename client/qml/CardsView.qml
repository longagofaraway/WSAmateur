import QtQuick 2.12
import QtQuick.Window 2.12

Rectangle {
    width: row.width + 20
    height: calculateViewHeight() + 20
    radius: 5
    border.width: 1
    color: "#564747"

    property ListModel listModel: ListModel {
        ListElement { type: "char"; level: 1; img: "imc"; }
        ListElement { type: "char"; level: 0; img: "imc0" }
        ListElement { type: "char"; level: 3; img: "imc3" }
        ListElement { type: "climax"; level: 0; img: "imc4" }
        ListElement { type: "char"; level: 1; img: "imc2" }
        ListElement { type: "char"; level: 1; img: "imc2" }
        ListElement { type: "char"; level: 1; img: "imc" }
        ListElement { type: "char"; level: 0; img: "imc0" }
        ListElement { type: "char"; level: 3; img: "imc3" }
        ListElement { type: "climax"; level: 0; img: "imc4" }
        ListElement { type: "char"; level: 1; img: "imc2" }
        ListElement { type: "char"; level: 1; img: "imc2" }
        ListElement { type: "char"; level: 1; img: "imc2" }
        ListElement { type: "char"; level: 1; img: "imc2" }
        ListElement { type: "char"; level: 1; img: "imc" }
        ListElement { type: "char"; level: 1; img: "imc2" }
        ListElement { type: "char"; level: 1; img: "imc2" }
        ListElement { type: "char"; level: 1; img: "imc" }
    }

    Row {
        id: row
        anchors {
            left: parent.left
            top: parent.top
            leftMargin: 10
            topMargin: 10
        }
        spacing: -root.cardWidth * 0.1

        CardsViewColumn {
            id: col1
            predicate: function(entry) {
                if (entry.level === 0 && entry.type === "char")
                    return true;
                return false;
            }
        }

        CardsViewColumn {
            id: col2
            predicate: function(entry) {
                if (entry.level === 1 && entry.type === "char")
                    return true;
                return false;
            }
        }
        CardsViewColumn {
            id: col3
            predicate: function(entry) {
                if ((entry.level === 2 || entry.level === 3) && entry.type === "char")
                    return true;
                return false;
            }
        }
        CardsViewColumn {
            id: col4
            predicate: function(entry) {
                if (entry.type === "climax" || entry.type === "event")
                    return true;
                return false;
            }
        }
    }

    function calculateViewHeight() {
        let max = col1.height;
        if (col2.height > max)
            max = col2.height;
        if (col3.height > max)
            max = col3.height;
        if (col4.height > max)
            max = col4.height;
        return max;
    }
}
