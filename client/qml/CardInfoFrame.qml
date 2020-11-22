import QtQuick 2.12
import QtQuick.Shapes 1.12

Shape {
    id: shape

    property ListModel effectsModel: ListModel {
        ListElement { effect: "Text is a 2019 Russian crime drama psychological thriller film directed by Klim Shipenko, an adaptation of the best-selling novel Text by writer Dmitry Glukhovsky, who adapted the novel into a movie script. The film stars Alexander Petrov, Ivan Yankovsky and Kristina Asmus." }
        ListElement { effect: "Text is a 2019 Russian crime drama psychological thriller film directed by Klim Shipenko, an adaptation of the best-selling novel Text by writer Dmitry Glukhovsky, who adapted the novel into a movie script. The film stars Alexander Petrov, Ivan Yankovsky and Kristina Asmus." }
    }

    property real selfHeight: effectsView.height + borderRect.borderWidth * 2

    z: 100
    width: 200
    height: effectsView.height + borderRect.borderWidth * 2
    ShapePath {
        strokeWidth: -1
        fillGradient: LinearGradient {
            x1: 0; y1: 0
            x2: shape.width / 5; y2: shape.height
            GradientStop { position: 0; color: "#383838" }
            GradientStop { position: 0.1; color: "#E7E7E7" }
            GradientStop { position: 0.2; color: "#383838" }
            GradientStop { position: 0.4; color: "#383838" }
            GradientStop { position: 0.5; color: "#E7E7E7" }
            GradientStop { position: 0.6; color: "#383838" }
            GradientStop { position: 0.9; color: "#383838" }
            GradientStop { position: 1.1; color: "#A5A5A5" }
            GradientStop { position: 1.2; color: "#383838" }
        }
        PathLine { x: shape.width; y: 0 }
        PathLine { x: shape.width; y: shape.height }
        PathLine { x: 0; y: shape.height }
        PathLine { x: 0; y: 0 }
    }

    Rectangle {
        id: borderRect
        property real borderWidth: 2

        color: "#FFFFFF"
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            bottom: parent.bottom

            leftMargin: borderWidth
            rightMargin: borderWidth
            topMargin: borderWidth
            bottomMargin: borderWidth
        }

        ListView {
            id: effectsView
            anchors.left: parent.left
            anchors.right: parent.right
            height: contentHeight
            model: effectsModel

            delegate: Component {
                Rectangle {
                    color: "#F2FFFE"
                    radius: 5
                    border.width: 1
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: effectText.contentHeight
                    Text {
                        id: effectText
                        width: parent.width
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignJustify
                        leftPadding: 4
                        rightPadding: 4
                        text: effect
                    }
                }
            }
        }
    }
}
