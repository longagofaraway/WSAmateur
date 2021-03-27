import QtQuick 2.12
import QtQml.Models 2.12
import QtQuick.Window 2.12
import QtGraphicalEffects 1.12
import QtQuick.Controls 1.1

import wsamateur 1.0

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

        property bool darker: false
        anchors.fill: backgroundImg
        source: backgroundImg
        color: darker ? "#D0000000" : "#B0000000"
    }

    Game {
        id: gGame
        property MulliganHeader mHeader
        property HelpText mHelpText

        anchors.fill: parent

        Hand{}
        MainButton {
            id: mainButton
            x: root.width * 0.75
            y: root.height * 0.85
        }

        MainButton {
            id: testButton
            x: root.width * 0.2
            y: root.height * 0.8
            mText: "test"
            state: "active"
            onClicked: {
                gGame.testAction();
            }
        }
        SequentialAnimation {
            id: waitAnim
            PauseAnimation { id: waitAnimPause; }
            ScriptAction { script: gGame.actionComplete() }
        }

        function getPlayer(opp = false) { return opp ? gGame.opponent : gGame.player; }

        function pause(pauseDuration) {
            gGame.startAction();
            waitAnimPause.duration = pauseDuration;
            waitAnim.start();
        }

        function createHelpText(mainText, subText) {
            let comp = Qt.createComponent("HelpText.qml");
            mHelpText = comp.createObject(gGame);
            mHelpText.mText = mainText;
            mHelpText.mHelpText = subText === undefined ? "" : subText;
        }

        function startHelpTextDestruction() {
            if (mHelpText === null)
                return;

            gGame.startUiAction();
            mHelpText.startDestroy();
        }

        function destroyHelpText() {
            mHelpText.destroy();
            mHelpText = null;
            gGame.uiActionComplete();
        }

        function startMulligan(firstTurn) {
            colorOverlay.darker = true;
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

            colorOverlay.darker = false;
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

        function endGame(victory) {
            mainButton.state = "waiting";
            let comp = Qt.createComponent("EndOfGame.qml");
            let indicator = comp.createObject(gGame);
            indicator.victory = victory;
            indicator.startAnimation();
        }

        function clockPhase() {
            createHelpText("Choose up to 1 card to send to Clock");
            mainButton.state = "active";
            mainButton.mText = "Next";
            mainButton.mSubText = "To Main Phase";
            mainButton.clicked.connect(clockPhaseFinished);
        }

        function clockPhaseFinished() {
            startHelpTextDestruction();
            mainButton.clicked.disconnect(clockPhaseFinished);
            mainButton.mFontSizeAdjustment = 0;
            mainButton.state = "waiting";

            gGame.sendClockPhaseFinished();
        }

        function mainPhase() {
            mainButton.state = "active";
            mainButton.mText = "Next";
            mainButton.mSubText = "To Attack";
            mainButton.clicked.connect(mainPhaseFinished);
        }

        function pauseMainPhase() {
            mainButton.clicked.disconnect(mainPhaseFinished);
            mainButton.state = "waiting";
        }

        function mainPhaseFinished() {
            mainButton.clicked.disconnect(mainPhaseFinished);
            mainButton.state = "waiting";

            gGame.sendMainPhaseFinished();
        }

        function attackDeclarationStep() {
            createHelpText("Choose attacker", "(Right click for Side Attack)");
            mainButton.state = "active";
            mainButton.mText = "Skip attack";
            mainButton.mSubText = "To Encore";
            mainButton.clicked.connect(skipAttack);
        }

        function skipAttack() {
            mainButton.clicked.disconnect(skipAttack);
            startHelpTextDestruction();
            mainButton.state = "waiting";

            gGame.sendEncoreCommand();
        }

        function attackDeclarationStepFinished() {
            startHelpTextDestruction();
            mainButton.state = "waiting";
        }

        function counterStep() {
            createHelpText("Counter step");
            mainButton.state = "active";
            mainButton.mText = "Take damage";
            mainButton.mSubText = "";
            mainButton.clicked.connect(sendTakeDamage);
        }

        function sendTakeDamage() {
            gGame.sendTakeDamageCommand();
        }

        function endCounterStep() {
            mainButton.clicked.disconnect(sendTakeDamage);
            startHelpTextDestruction();
            mainButton.state = "oppTurn";
        }

        function showText(mainText, subText) {
            createHelpText(mainText, subText);
        }

        function hideText() { startHelpTextDestruction(); }

        function encoreStep() {
            createHelpText("Choose characters to encore");
            mainButton.state = "active";
            mainButton.mText = "End turn";
            mainButton.mSubText = "";
            mainButton.clicked.connect(endTurn);
        }

        function pauseEncoreStep() {
            mainButton.clicked.disconnect(endTurn);
            startHelpTextDestruction();
            mainButton.state = "waiting";
        }

        function endTurn() {
            mainButton.clicked.disconnect(endTurn);
            gGame.sendEndTurn();
            startHelpTextDestruction();
            mainButton.state = "waiting";
        }

        function discardTo7() {
            createHelpText("Discard cards from your hand down to 7");
            mainButton.state = "waiting";
        }

        function clearHelpText() {
            startHelpTextDestruction();
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
    }
}
