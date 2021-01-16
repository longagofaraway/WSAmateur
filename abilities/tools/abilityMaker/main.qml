import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12

import abilityMaker 1.0

Window {
    visible: true
    width: 640
    height: 480
    title: qsTr("Ability Maker")


    AbilityMaker {
        id: root

        property int abilityType
        property string ability

        anchors.fill: parent

        ComboBox {
            id: rootCombo
            anchors.horizontalCenter: root.horizontalCenter
            currentIndex: -1
            model: ["Cont", "Auto", "Act", "Event"]
            onCurrentIndexChanged: {
                enabled = false;
                root.abilityType = currentIndex + 1;
                let ab = "";
                switch (currentIndex) {
                case 0:
                    ab = "ContAbility";
                    break;
                case 1:
                    ab = "AutoAbility";
                    break;
                case 2:
                    ab = "ActAbility";
                    break;
                case 3:
                    ab = "EventAbility";
                    break;
                }

                root.ability = root.createComponent(ab);
            }
        }
        Button {
            anchors.left: rootCombo.right
            anchors.leftMargin: 5
            text: "Create ability"
            onClicked: {
                root.createAbility(root.abilityType, root.ability);
            }
        }
    }
}
