import QtQuick 2.12
import QtQuick.Controls 2.12

Row {
    signal addComponent()
    signal removeComponent()

    spacing: 5
    height: childrenRect.height
    property int currentNumber: 1

Button {
    width: 80
    height: 30
    text: "+"
    onClicked: {
        currentNumber = currentNumber + 1;
        addComponent();
    }
}


CancelButton {
    width: 50
    height: 30
    onClicked: {
        currentNumber = currentNumber - 1;
        removeComponent();
    }
    visible: currentNumber > 1
}

}
