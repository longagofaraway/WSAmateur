import QtQuick 2.0

Rectangle {
    id: menuItem

    signal menuClicked()
    property string text: ""
    property bool active: false
    property var activeGradient: Gradient {
        GradientStop { position: 0.0; color: "#60000000" }
        GradientStop { position: 0.3; color: "#50333333" }
        GradientStop { position: 0.5; color: "#50888888" }
        GradientStop { position: 0.7; color: "#50333333" }
        GradientStop { position: 1.0; color: "#60000000" }
    }
    property var inactiveGradient: Gradient {
        GradientStop { position: 0.0; color: "#50000000" }
        GradientStop { position: 1.0; color: "#50000000" }
    }
    property var hoverGradient: Gradient {
        GradientStop { position: 0.0; color: "#50000000" }
        GradientStop { position: 0.42; color: "#50000000" }
        GradientStop { position: 0.5; color: "#50777777" }
        GradientStop { position: 0.58; color: "#50000000" }
        GradientStop { position: 1.0; color: "#50000000" }
    }

    gradient: sideMenu.activeGradient
    Component.onCompleted: {
        menuItem.gradient = active ? menuItem.activeGradient : menuItem.inactiveGradient
    }

    Text {
        anchors.centerIn: parent
        text: menuItem.text
        color: "white"
        font.pointSize: 36
    }

    MouseArea {
        enabled: !active
        anchors.fill: parent
        hoverEnabled: true
        onEntered: menuItem.gradient = menuItem.hoverGradient
        onExited: menuItem.gradient = menuItem.inactiveGradient
        onClicked: menuItem.menuClicked()
    }
}
