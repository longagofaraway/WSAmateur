#pragma once

#include <deque>

#include <QQuickItem>

#include "client.h"
#include "userListModel.h"

class EventInvitedToPlay;
class EventInviteWithdrawn;
class EventInviteDeclined;

class Lobby : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(UserListModel *userListModel READ userListModel CONSTANT FINAL)
private:
    UserListModel model;
    Client *client;
    std::deque<UserInfo> inviteQueue;
    bool isInvited = false;

public:
    Lobby();
    ~Lobby();

    void init(Client *client_);
    UserListModel* userListModel() { return &model; }

    Q_INVOKABLE void joinQueue();
    Q_INVOKABLE bool canInvite(int row);
    Q_INVOKABLE void sendInvite();
    Q_INVOKABLE void cancelInvite();
    Q_INVOKABLE void refuseInvite();
    Q_INVOKABLE void acceptInvite();

signals:
    void lobbyCreated();
    void userCountChanged(int userCount);
    void inviteSent();
    void inviteReceived(QString userName);
    void inviteWithdrawn();
    void inviteDeclined();

private:
    void lobbyInfoReceived(EventLobbyInfo &event);
    void playerInviteReceived(const EventInvitedToPlay &event);
    void playerInviteWithdrawn(const EventInviteWithdrawn &event);
    void playerInviteDeclined(const EventInviteDeclined& event);

private slots:
    void processLobbyEvent(const std::shared_ptr<LobbyEvent> event);

protected:
    void componentComplete() override;
};

