#pragma once

#include <optional>

#include <QAbstractTableModel>
#include <QQuickItem>

#include <userInfo.pb.h>

class UserListModel : public QAbstractTableModel
{
    Q_OBJECT
private:
    std::vector<UserInfo> userList;
    int selectedRow = -1;
    int selectedPlayerId = -1;

public:
    enum TableRoles {
        TableDataRole = Qt::UserRole + 1,
        SelectedRole
    };
    Q_ENUM(TableRoles)

    UserListModel();

    Q_INVOKABLE void select(int row);

    int idByRow(int row);
    std::optional<int> selectedId() const;

    void update(std::vector<UserInfo>&& newUserList);

    int rowCount(const QModelIndex & = QModelIndex()) const override;
    int columnCount(const QModelIndex & = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
};

