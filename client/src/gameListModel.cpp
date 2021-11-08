#include "gameListModel.h"

#include "playerInfo.pb.h"

GameListModel::GameListModel() {
    UserInfo userInfo;
    userInfo.set_name("petr");
    userList.push_back(userInfo);
    userInfo.set_name("vasilisa");
    userList.push_back(userInfo);
    for (int i = 0; i < 20; i++)
        userList.push_back(userInfo);
}

int GameListModel::rowCount(const QModelIndex &) const {
    return static_cast<int>(userList.size());
}

int GameListModel::columnCount(const QModelIndex &) const {
    return 1;
}

QVariant GameListModel::data(const QModelIndex &index, int role) const {
    switch (role) {
    case TableDataRole: {
        if (static_cast<size_t>(index.row()) >= userList.size())
            return QVariant();
        const auto &userInfo = userList[index.row()];
        switch (index.column()) {
        case 0:
            return QString::fromStdString(userInfo.name());
        }
    }
    default:
        break;
    }

    return QVariant();
}

QVariant GameListModel::headerData(int section, Qt::Orientation orientation, int role) const {
    static QString playerName = "Lobby";
    switch (section) {
    case 0:
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
