#pragma once

#include <memory>

#include <QQuickItem>
#include <QThread>

#include "client.h"

class Game;
class Lobby;
class EventServerHandshake;
class EventDatabase;
class EventImageLinks;
class Updater;
class PublicServers;

class WSApplication : public QQuickItem
{
    Q_OBJECT
private:
    QThread clientThread;
    Client* client = nullptr;
    int playerId;
    bool connectionFailed = false;
    PublicServers *publicServers = nullptr;
    Updater *updater = nullptr;

    bool dbIsUpToDate = true;
    bool imageLinksAreUpToDate = true;

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
    void connectToHost(QString address, QString port);

private:
    void processHandshake(const EventServerHandshake &event);
    void updateDatabase(const EventDatabase &event);
    void updateImageLinks(const EventImageLinks &event);
    void sendDatabaseRequest();
    void sendImageLinksRequest();
    void tryEnterLobby();
    void gameJoined(const EventGameJoined &event);
    void userIdentification();
    void connectToServer();
    bool checkImageLinksFile();
    bool checkUsername();
    bool clientIsUpToDate() const;

    void startUpdater(const std::string& neededVersion);
};

