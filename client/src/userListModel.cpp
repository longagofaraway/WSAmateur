#include "userListModel.h"

#include "playerInfo.pb.h"

UserListModel::UserListModel() {}

void UserListModel::select(int row) {
    if (static_cast<size_t>(row) >= userList.size())
        return;

    int oldSelectedRow = selectedRow;
    selectedRow = row;
    selectedPlayerId = userList.at(row).id();

    // deselect
    if (oldSelectedRow >= 0) {
        auto modelIndex = index(oldSelectedRow, 0);
        emit dataChanged(modelIndex, modelIndex, { SelectedRole });
    }

    auto modelIndex = index(row, 0);
    emit dataChanged(modelIndex, modelIndex, { SelectedRole });
}

int UserListModel::idByRow(int row) {
    return userList.at(row).id();
}

std::optional<int> UserListModel::selectedId() const {
    if (selectedPlayerId == -1)
        return std::nullopt;
    return selectedPlayerId;
}

void UserListModel::update(std::vector<UserInfo> &&newUserList) {
    beginResetModel();
    userList.swap(newUserList);
    endResetModel();

    bool userFound = false;
    for (size_t i = 0; i < userList.size(); ++i) {
        if (userList[i].id() != selectedPlayerId)
            continue;
        selectedRow = i;
        userFound = true;
        break;
    }

    if (!userFound) {
        selectedRow = -1;
        selectedPlayerId = -1;
    }
}

int UserListModel::rowCount(const QModelIndex &) const {
    return static_cast<int>(userList.size());
}

int UserListModel::columnCount(const QModelIndex &) const {
    return 1;
}

QVariant UserListModel::data(const QModelIndex &index, int role) const {
    switch (role) {
    case TableDataRole: {
        if (static_cast<size_t>(index.row()) >= userList.size())
            return QVariant();
        const auto &userInfo = userList[index.row()];
        switch (index.column()) {
        case 0:
            return QString::fromStdString(userInfo.name());
        }
        break;
    }
    case SelectedRole:
        return index.row() == selectedRow;
    default:
        break;
    }

    return QVariant();
}

QVariant UserListModel::headerData(int section, Qt::Orientation orientation, int role) const {
    static QString playerName = "Lobby";
    switch (section) {
    case 0:
        return playerName;
    }
    return QVariant();
}

QHash<int, QByteArray> UserListModel::roleNames() const {
    static QHash<int, QByteArray> *roles;
    if (!roles) {
        roles = new QHash<int, QByteArray>;
        (*roles)[TableDataRole] = "tableData";
        (*roles)[SelectedRole] = "selected";
    }
    return *roles;
}
