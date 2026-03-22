import QtQuick 2.12
import QtQuick.Controls 2.12


Button {
    signal openView()

    function setDescription(description: string) {
        text = "Open"+'\n'+description
    }

    text: "Open"
    onClicked: {
        openView();
    }
}
