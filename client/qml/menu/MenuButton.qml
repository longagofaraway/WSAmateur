import QtQuick 2.0

Rectangle {
    id: btn

    property bool active: true
    property alias text: buttonText.text
    property string activeColor: "#22162A"
    signal pressed()

    width: 158//231
    height: 50//72
    radius: 3
    color: "#22162A"

    states: [
        State {
            name: "active"
            when: btn.active
            PropertyChanges { target: btn; color: btn.activeColor }
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
        onPressed: btn.color = "red"
        onReleased: btn.color = "#22162A"
        onClicked: btn.pressed()
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
