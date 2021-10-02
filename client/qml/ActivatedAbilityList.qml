import QtQuick 2.0
import QtQml.Models 2.12

import wsamateur 1.0

ListView {
    id: lview

    property bool mOpponent
    property ActivatedAbilityModel mModel: innerModel

    z: 160
    x: gGame.width * (mOpponent ? 0.71 : 0.12)
    y: {
        if (mOpponent)
            return gGame.height * 0.2;
        return (gGame.height - contentHeight) / 2;
    }
    width: 300
    height: contentHeight
    interactive: false
    spacing: 5
    model: mModel
    delegate: ActivatedAbility {}

    add: Transition {
        NumberAnimation { property: "x"; from: gGame.mapToItem(lview, gGame.width / 2, 0).x; duration: 200 }
        NumberAnimation {
            property: "y";
            from: gGame.mapToItem(lview, 0, gGame.height / 4 * (mOpponent ? 1 : 3) - 90).y;
            duration: 200
        }
        NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 200 }
    }
    displaced: Transition {
        NumberAnimation { properties: "x,y"; duration: 400; easing.type: Easing.OutBounce }
    }
    Behavior on y { NumberAnimation { duration: 200 } }
    remove: Transition {
        id: removeTrans
        NumberAnimation { property: "x"; to: -100; duration: 200 }
        NumberAnimation { property: "y"; from: 0; to: -removeTrans.ViewTransition.item.height / 2; duration: 200 }
        NumberAnimation { property: "opacity"; from: 1; to: 0; duration: 200 }
    }

    function hightlightCard(row, value) {
        mModel.highlightCorrespondingCard(row, value);
    }
}
