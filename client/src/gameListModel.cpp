#include "gameListModel.h"

#include "playerInfo.pb.h"

GameListModel::GameListModel() {
    GameInfo gameInfo;
    gameInfo.set_name("game 1");
    PlayerInfo playerInfo;
    playerInfo.set_name("loh");
    auto newPlayer = gameInfo.add_players();
    *newPlayer = playerInfo;
    gameList.push_back(gameInfo);
    gameInfo.set_name("game 2");
    playerInfo.set_name("loh 2");
    *newPlayer = playerInfo;
    gameList.push_back(gameInfo);
    for (int i = 0; i < 20; i++)
        gameList.push_back(gameInfo);
}

int GameListModel::rowCount(const QModelIndex &) const {
    return static_cast<int>(gameList.size());
}

int GameListModel::columnCount(const QModelIndex &) const {
    return 2;
}

QVariant GameListModel::data(const QModelIndex &index, int role) const {
    switch (role) {
    case TableDataRole: {
        if (static_cast<size_t>(index.row()) >= gameList.size())
            return QVariant();
        const auto &gameInfo = gameList[index.row()];
        switch (index.column()) {
        case 0:
            return QString::fromStdString(gameInfo.name());
        case 1:
            if (gameInfo.players_size())
                return QString::fromStdString(gameInfo.players(0).name());
        }
    }
    default:
        break;
    }

    return QVariant();
}

QVariant GameListModel::headerData(int section, Qt::Orientation orientation, int role) const {
    static QString gameName = "Game name";
    static QString playerName = "Player name";
    switch (section) {
    case 0:
        return gameName;
    case 1:
        return playerName;
    }
    return QVariant();
}

QHash<int, QByteArray> GameListModel::roleNames() const
{
    static QHash<int, QByteArray> *roles;
    if (!roles) {
        roles = new QHash<int, QByteArray>;
        (*roles)[TableDataRole] = "tableData";
    }
    return *roles;
}
