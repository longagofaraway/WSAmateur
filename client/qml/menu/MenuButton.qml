import QtQuick 2.0

Rectangle {
    id: btn

    property bool active: true
    property alias mText: buttonText.text

    width: 158//231
    height: 50//72
    radius: 3
    color: "#22162A"

    states: [
        State {
            name: "active"
            when: btn.active
            PropertyChanges { target: btn; color: "#22162A" }
            PropertyChanges { target: mouse; enabled: true }
        },
        State {
            name: "inactive"
            when: !btn.active
            PropertyChanges { target: btn; color: "#343434" }
            PropertyChanges { target: mouse; enabled: false }
        }
    ]

    MouseArea {
        id: mouse
        anchors.fill: parent
        hoverEnabled: true
        onPressed: btn.color = "red"
        onReleased: btn.color = "#22162A"
    }

    Text {
        id: buttonText
        anchors.centerIn: parent
        text: ""
        color: "white"
        font.pointSize: 18
    }

    function setInactive() {
        if (btn.state != "inactive")
            btn.state = "inactive";
    }

    function setActive() {
        if (btn.state != "active")
            btn.state = "active";
    }
}
