#pragma once

#include <QQuickItem>

#include "userListModel.h"

class Lobby : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(UserListModel *userListModel READ userListModel CONSTANT FINAL)
private:
    UserListModel model;

public:
    Lobby();

    UserListModel* userListModel() { return &model; }
};

