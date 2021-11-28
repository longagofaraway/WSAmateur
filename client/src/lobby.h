#pragma once

#include <QQuickItem>

#include "client.h"
#include "userListModel.h"

class Lobby : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(UserListModel *userListModel READ userListModel CONSTANT FINAL)
private:
    UserListModel model;
    Client *client;

public:
    Lobby();

    void init(Client *client_);
    UserListModel* userListModel() { return &model; }

    Q_INVOKABLE void joinQueue();
    Q_INVOKABLE bool canInvite(int row);

signals:
    void lobbyCreated();
    void userCountChanged(int userCount);

private:
    void lobbyInfoReceived(EventLobbyInfo &event);

private slots:
    void processLobbyEvent(const std::shared_ptr<LobbyEvent> event);

protected:
    void componentComplete() override;
};

