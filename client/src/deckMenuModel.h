#pragma once

#include <QAbstractTableModel>
#include <QQuickItem>

struct DeckMenuItem {
    QString name;
    QString thumbnail = "cardback";
};

class DeckMenuModel : public QAbstractTableModel
{
    Q_OBJECT
private:
    std::vector<DeckMenuItem> decks;
    static constexpr int kColumnCount = 4;

public:
    enum TableRoles {
        NameRole = Qt::UserRole + 1,
        ThumbnailRole,
        LastElementRole,
        InvalidElementRole
    };
    Q_ENUM(TableRoles)

    DeckMenuModel();

    void setDecks(std::vector<DeckMenuItem> &&newData);
    void addDeck(DeckMenuItem &&deck);

    int rowCount(const QModelIndex & = QModelIndex()) const override;
    int columnCount(const QModelIndex & = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
};

