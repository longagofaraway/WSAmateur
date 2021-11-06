#pragma once

#include <QQuickItem>

#include "gameListModel.h"

class Lobby : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(GameListModel *gameListModel READ gameListModel CONSTANT FINAL)
private:
    GameListModel model;

public:
    Lobby();

    GameListModel* gameListModel() { return &model; }
};

