import QtQuick 2.0
import QtQml.Models 2.12

import wsamateur.abilityModel 1.0

ListView {
    id: lview

    property bool mOpponent
    property AbilityModel mModel: innerModel

    z: 150
    x: 280
    y: mOpponent ? 200 : (root.height / 2)
    width: 300
    height: contentHeight
    model: mModel
    delegate: ActivatedAbility {
        mAbilityText: model.text
        mSource: model.code
        mBtnActive: model.btnActive
        mBtnText: model.btnText
    }

    add: Transition {
        NumberAnimation { property: "x"; from: gGame.mapToItem(lview, gGame.width / 2, 0).x; duration: 200 }
        NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 200 }
    }
    displaced: Transition {
        NumberAnimation { properties: "x,y"; duration: 400; easing.type: Easing.OutBounce }
    }
}
