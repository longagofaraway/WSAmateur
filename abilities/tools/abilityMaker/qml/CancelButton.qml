import QtQuick 2.12
import QtQuick.Controls 2.12

RoundButton {
    x: 3 * root.width / 4
    text: "\u2717" // Unicode Character 'CROSS MARK'
    palette {
        button: "#FFB3B3"
    }
}
