#pragma once

#include <memory>

#include <QQuickItem>
#include <QThread>

#include "client.h"

class Game;
class Lobby;
class EventServerHandshake;
class EventDatabase;
class Updater;

class WSApplication : public QQuickItem
{
    Q_OBJECT
private:
    QThread clientThread;
    Client* client = nullptr;
    int playerId;
    bool connectionFailed = false;
    Updater *updater = nullptr;

    enum class InitPhase {
        ImageFileLinks,
        Username,
        Done
    };
    InitPhase initPhase = InitPhase::ImageFileLinks;

public:
    WSApplication();
    ~WSApplication();

    Q_INVOKABLE void startInitialization();
    Q_INVOKABLE void initGame(Game *game);
    Q_INVOKABLE void initLobby(Lobby *lobby);
    Q_INVOKABLE void imageLinksFileChosen(QString path);
    Q_INVOKABLE void setUsername(QString name);
    Q_INVOKABLE void gameEnded();

signals:
    void startGame();
    void needUpdate();
    void loadLobby();
    void imageFileParsed();
    void usernameSet();

    void error(QString message);
    void imageLinksFileNotFound();
    void usernameNotFound();
    void imageFileParseError();

    void progressMade(qint64 bytesRead, qint64 totalBytes);
    void startUpdate();

private slots:
    void processSessionEvent(const std::shared_ptr<SessionEvent> event);
    void processLobbyEvent(const std::shared_ptr<LobbyEvent> event);
    void onConnectionClosed();
    void initialization();

private:
    void processHandshake(const EventServerHandshake &event);
    void updateDatabase(const EventDatabase &event);
    void sendDatabaseRequest();
    void enterLobby();
    void gameJoined(const EventGameJoined &event);
    void userIdenditification();
    void connectToServer();
    bool checkImageLinksFile();
    bool checkUsername();

    void startUpdater(const std::string& neededVersion);
};

