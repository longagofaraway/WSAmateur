import QtQuick 2.12
import QtQml.Models 2.12
import QtQuick.Window 2.12
import QtGraphicalEffects 1.12
import QtQuick.Controls 1.1

import wsamateur.game 1.0

Item {
    id: root

    property real cardWidth: width * 0.0677
    property real cardHeight: height * 0.1685
    property var stageDropTarget: undefined

    GaussianBlur {
        id: blurEffect
        z: 1
        anchors.fill: parent
        source: blurTarget

        radius: 16
        samples: 32

        opacity: 0
        Behavior on opacity { NumberAnimation { duration: 100 } }
    }

    Item {
        id: blurTarget
        anchors.fill: parent
    Image {
        id: backgroundImg
        anchors.fill: parent
        source: "qrc:///resources/background.jpg"
        fillMode: Image.PreserveAspectCrop
    }

    ColorOverlay {
        id: colorOverlay
        anchors.fill: backgroundImg
        source: backgroundImg
        color: "#B0000000"
    }

    Game {
        id: gGame
        property MulliganHeader mHeader
        property HelpText mHelpText

        anchors.fill: parent

        MainButton {
            id: mainButton
            x: root.width * 0.75
            y: root.height * 0.85
            onClicked: {

            }
        }

        function changeState() {
            if (mainButton.state === "")
                mainButton.state = "active";
            else if (mainButton.state === "active")
                mainButton.state = "oppTurn";
            else
                mainButton.state = "";
        }

        function destroyHelpText() {
            mHelpText.destroy();
            mHelpText = null;
        }

        function startMulligan(firstTurn) {
            colorOverlay.color = "#D0000000";
            blurEffect.opacity = 1;

            var comp = Qt.createComponent("MulliganHeader.qml");
            mHeader = comp.createObject(root);
            mHeader.firstTurn = firstTurn;
            mHeader.finished.connect(mulliganFinished);
        }

        function changeCardCountForMulligan(selected) {
            mHeader.cardSelected(selected);
        }
        function changeCardCountForClock(selected) {
            if (selected) {
                mainButton.mText = "Send To Clock";
                mainButton.mFontSizeAdjustment = -3;
                mainButton.mSubText = "";
            } else {
                mainButton.mText = "Next";
                mainButton.mFontSizeAdjustment = 0;
                mainButton.mSubText = "To Main Phase";
            }
        }

        function mulliganFinished() {
            mHeader.finished.disconnect(mulliganFinished);

            colorOverlay.color = "#B0000000";
            blurEffect.opacity = 0;
            mHeader.destroy();
            mHeader = null;

            gGame.sendMulliganFinished();
        }

        function startTurn(opponent) {
            if (opponent)
                mainButton.state = "oppTurn";
            else
                mainButton.state = "waiting";
            let comp = Qt.createComponent("TurnIndicator.qml");
            let indicator = comp.createObject(gGame);
            indicator.opponent = opponent;
            indicator.startAnimation();
        }

        function clockPhase() {
            let comp = Qt.createComponent("HelpText.qml");
            mHelpText = comp.createObject(gGame);
            mHelpText.mText = "Choose up to 1 card to send to Clock";
            mainButton.state = "active";
            mainButton.mText = "Next";
            mainButton.mSubText = "To Main Phase";
            mainButton.clicked.connect(clockPhaseFinished);
        }

        function clockPhaseFinished() {
            mHelpText.startDestroy();
            mainButton.clicked.disconnect(clockPhaseFinished);
            mainButton.mFontSizeAdjustment = 0;
            mainButton.state = "waiting";

            gGame.sendClockPhaseFinished();
        }

        function mainPhase() {
            mainButton.state = "active";
            mainButton.mText = "Next";
            mainButton.mSubText = "To Attack";
        }
    }

    Button {
        id: exit
        text: "exit"
        onClicked: Qt.quit()
    }
    Button {
        id: minimize
        anchors.top: exit.bottom
        text: "exit fullscreen" + root.height
        onClicked: mainWindow.visibility = "Windowed"
    }
    Button {
        id: maximize
        anchors.top: minimize.bottom
        text: "enter fullscreen" + root.width
        onClicked: mainWindow.visibility = "FullScreen"
    }
    Button {
        id: deleter
        anchors.top: maximize.bottom
        text: "delete card"
        onClicked: handView.model.model.remove(0)
    }
    Button {
        id: mover
        anchors.top: deleter.bottom
        text: "move card"
        onClicked: handView.model.items.move(2, 5)
    }
    Row {
        id: drawer
        anchors.top: mover.bottom
        Button {
            text: "draw card"
            onClicked: drawCard()
        }
        Button {
            text: "draw opponent"
            onClicked: drawOpponentCard()
        }
    }
    Row {
        id: stocker
        anchors.top: drawer.bottom
        Button {
            text: "card to stock"
            onClicked: toStock()
        }
        Button {
            text: "opp card to stock"
            onClicked: toOpponentStock()
        }
    }
    Button {
        id: glower
        anchors.top: stocker.bottom
        text: "glow card"
        onClicked: {
            glowing = !glowing;
            handView.model.model.setProperty(5, "glow", glowing);
        }
    }
    Row {
        anchors.top: glower.bottom
        Button {
            id: wrAdder
            text: "add to wr"
            onClicked: {
                var imgs = [{ type: "char", level: 1, img: "qrc:///resources/images/imc" }, { type: "climax", level: 0, img: "qrc:///resources/images/imc4" },
                            { type: "char", level: 0, img: "qrc:///resources/images/imc0" }, { type: "char", level: 3, img: "qrc:///resources/images/imc3" }];
                var img = imgs[Math.floor(Math.random() * 10) % 4];
                deck.view.listModel.append(img);
            }
        }
        Button {
            text: "hand to wr"
            onClicked: handToWr()
        }
    }

    property bool glowing: true
    property int testIndex: 12
    function drawOpponentCard() {
        oppHandView.model.model.insert(0, { src: "", code: "q"+String(testIndex) });
        testIndex += 1;
        var newIndex = 0;
        var newCardInHand = handView.model.items.create(newIndex);
        var comp = Qt.createComponent("TopDeck.qml");
        var topDeck = comp.createObject(root, {opponent: true, cardHandModelIndex: newIndex});
        var point = root.mapFromItem(oppHandView.contentItem, newCardInHand.x, newCardInHand.y);
        topDeck.startOpponentAnimation("hand", point.x, point.y);
    }

    function drawCard() {
        handView.model.model.append({ src: "", code: "q"+String(testIndex) });
        testIndex += 1;
        var newIndex = handView.model.items.count - 1;
        var newCardInHand = handView.model.items.create(newIndex);
        var comp = Qt.createComponent("TopDeck.qml");
        var topDeck = comp.createObject(root, {cardHandModelIndex: newIndex});
        var imgs = ["qrc:///resources/images/imc", "qrc:///resources/images/imc2", "qrc:///resources/images/imc3"];
        topDeck.img = imgs[Math.floor(Math.random() * 10) % 3];
        var point = root.mapFromItem(handView.contentItem, newCardInHand.x, newCardInHand.y);
        topDeck.startPlayerAnimation("hand", point.x, point.y);
    }
    function addPlayerStock() {
        stock.model.append({code: "q1"});
    }
    function addOpponentStock() {
        oppStock.model.append({code: "q1"});
    }

    function toStock() {
        var destY = stock.y + stock.getStockHeight() - root.cardWidth;
        var comp = Qt.createComponent("TopDeck.qml");
        var topDeck = comp.createObject(root, {source: "qrc:///resources/images/cardback"});
        topDeck.img = "qrc:///resources/images/imc2";
        topDeck.startToStockAnim("stock", stock.x, destY, addPlayerStock);
    }
    function toOpponentStock() {
        var destY = oppStock.y - root.cardHeight * 0.2;
        var comp = Qt.createComponent("TopDeck.qml");
        var topDeck = comp.createObject(root, {opponent: true, img: "qrc:///resources/images/imc2"});
        topDeck.startToStockAnim("stock", oppStock.x, destY, addOpponentStock);
    }
    function handToWr() {

    }

    /*StagePlace { row: 1 }
    StagePlace { row: 1; anchors.horizontalCenterOffset: parent.width * 0.14 }
    StagePlace { row: 1; anchors.horizontalCenterOffset: -parent.width * 0.14 }
    StagePlace { row: 2; anchors.horizontalCenterOffset: parent.width * 0.07 }
    StagePlace { row: 2; anchors.horizontalCenterOffset: -parent.width * 0.07 }

    StagePlace { opponent: true; row: 1 }
    StagePlace { opponent: true; row: 1; anchors.horizontalCenterOffset: parent.width * 0.14 }
    StagePlace { opponent: true; row: 1; anchors.horizontalCenterOffset: -parent.width * 0.14 }
    StagePlace { opponent: true; row: 2; anchors.horizontalCenterOffset: parent.width * 0.07 }
    StagePlace { opponent: true; row: 2; anchors.horizontalCenterOffset: -parent.width * 0.07 }*/

    /*Stock {
        id: oppStock
        opponent: true
    }
    Stock {
        id: stock
    }*/

    /*Item {
        z: 3
        width: 700
        height: 200
        x: root.width / 2 - width / 2;
        y: root.height / 2 - height / 2;
        scale: 0

        RadialGradient {
            anchors.fill: parent
            horizontalRadius: 700
            verticalRadius: 100
            gradient: Gradient {
                GradientStop { position: 0.0; color: "#FCDE01" }
                GradientStop { position: 0.5; color: "#00FCDE01" }
            }
        }
        Text {
            anchors.centerIn: parent
            text: "Your Turn"
            color: "white"
            font.pointSize: 60
            font.bold: true
        }

        Component.onCompleted: {
            scaleAnim.start();
        }

        NumberAnimation on scale { id: scaleAnim; to: 1; duration: 300; }
    }*/

    /*Card {
        id: deck

        property CardsView view

        source: "qrc:///resources/images/cardback"
        z: 1

        anchors { right: parent.right; rightMargin: parent.width * 0.05;
                  top: parent.verticalCenter; topMargin: parent.height * 0.03 }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                let comp = Qt.createComponent("CardsView.qml");
                let incubator = comp.incubateObject(deck, { visible: false });
                let createdCallback = function(status) {
                    if (status === Component.Ready) {
                        deck.view = incubator.object;
                        deck.view.anchors.right = deck.left;
                        deck.view.visible = true;
                    }
                }
                if (incubator.status !== Component.Ready) {
                    incubator.onStatusChanged = createdCallback;
                } else {
                    createdCallback(Component.Ready);
                }
            }
        }
    }*/

    /*Text {
        id: texthere
        anchors.centerIn: parent
        text: String(handView.x) + " " + String(handView.y) + "\n" + String(handView.width)+" " + String(handView.height)
        color: "white"
    }*/
    }
}
