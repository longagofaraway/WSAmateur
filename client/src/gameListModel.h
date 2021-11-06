#pragma once

#include <QAbstractTableModel>
#include <QQuickItem>

#include <gameInfo.pb.h>

class GameListModel : public QAbstractTableModel
{
    Q_OBJECT
private:
    std::vector<GameInfo> gameList;

public:
    enum TableRoles {
        TableDataRole = Qt::UserRole + 1
    };
    Q_ENUM(TableRoles)

    GameListModel();

    int rowCount(const QModelIndex & = QModelIndex()) const override;
    int columnCount(const QModelIndex & = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
};

