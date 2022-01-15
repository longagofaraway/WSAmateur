#pragma once

#include <memory>

#include <QQuickItem>
#include <QThread>

#include "client.h"

class Game;
class Lobby;
class EventServerHandshake;
class EventDatabase;

class WSApplication : public QQuickItem
{
    Q_OBJECT
private:
    QThread clientThread;
    Client* client = nullptr;
    int playerId;
    bool connectionFailed = false;

    enum class InitPhase {
        ImageFileLinks,
        Username,
        Done
    };
    InitPhase initPhase = InitPhase::ImageFileLinks;

public:
    WSApplication();
    ~WSApplication();

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

    void error();
    void imageLinksFileNotFound();
    void usernameNotFound();
    void imageFileParseError();

private slots:
    void processSessionEvent(const std::shared_ptr<SessionEvent> event);
    void processLobbyEvent(const std::shared_ptr<LobbyEvent> event);
    void onConnectionClosed();

protected:
    void componentComplete() override;

private:
    void initialization();
    void processHandshake(const EventServerHandshake &event);
    void updateDatabase(const EventDatabase &event);
    void sendDatabaseRequest();
    void enterLobby();
    void gameJoined(const EventGameJoined &event);
    void userIdenditification();
    void connectToServer();
    bool checkImageLinksFile();
    bool checkUsername();
};

