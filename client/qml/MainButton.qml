import QtQuick 2.12
import QtGraphicalEffects 1.12

Item {
    id: btn
    width: 210//696
    height: 75//247

    signal clicked()
    property string mText
    property int mFontSizeAdjustment
    property string mSubText

    states: [
        State {
            name: "active"
            PropertyChanges { target: btnNormal; visible: true }
            PropertyChanges { target: glow; visible: true }
            PropertyChanges { target: mouse; enabled: true }
            PropertyChanges { target: buttonText; visible: true }
            PropertyChanges { target: inactiveRect; visible: false }
            PropertyChanges { target: subTextRect; visible: mSubText }
        },
        State {
            name: "oppTurn"
            PropertyChanges { target: oppText; visible: true }
            PropertyChanges { target: hourglass; visible: false }
        }
    ]
    Image {
        id: btnNormal
        anchors.fill: parent
        source: "qrc:///resources/images/btn_state"
        visible: false
    }

    MouseArea {
        id: mouse
        enabled: false
        anchors.fill: parent
        onPressed: btnNormal.source = "qrc:///resources/images/btn_pressed"
        onReleased: btnNormal.source = "qrc:///resources/images/btn_state"
        onClicked: btn.clicked()
    }

    Glow {
        id: glow
        anchors.fill: btnNormal
        color: "white"
        source: btnNormal
        spread: 0.3
        visible: false
    }

    Text {
        id: buttonText
        visible: false
        //anchors { top: parent.top; topMargin: 15 }
        anchors.centerIn: parent
        anchors.verticalCenterOffset: -5
        width: parent.width
        text: mText
        horizontalAlignment: Text.AlignHCenter
        color: "white"
        font.pointSize: 20 + mFontSizeAdjustment
    }

    Rectangle {
        id: inactiveRect
        anchors.fill: parent
        radius: 35
        color: "#A0000000"
        Image {
            id: hourglass
            anchors.centerIn: parent
            source: "qrc:///resources/images/hourglass"
            width: parent.height * 0.5
            height: parent.height * 0.67
        }
        Text {
            id: oppText
            visible: false
            anchors.centerIn: parent
            width: parent.width
            text: "Opponent's Turn"
            horizontalAlignment: Text.AlignHCenter
            color: "white"
            font.pointSize: 18
        }
    }

    Rectangle {
        id: subTextRect
        visible: false
        anchors { top: parent.bottom; topMargin: 3; horizontalCenter: parent.horizontalCenter }
        width: subText.contentWidth + 12; height: subText.height
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0.0; color: "#000F0F0F" }
            GradientStop { position: 0.1; color: "#0F0F0F" }
            GradientStop { position: 0.9; color: "#0F0F0F" }
            GradientStop { position: 1.0; color: "#000F0F0F" }
        }

        Text {
            id: subText
            anchors.centerIn: parent
            text: mSubText
            color: "white"
            font.pointSize: 12
        }
    }

    function setText(text) {
        buttonText.text = text;
    }
}
