#pragma once

#include <vector>

#include <QAbstractListModel>

#include "card.h"

class CardModel : public QAbstractListModel
{
    Q_OBJECT
private:
    std::vector<Card> mCards;

public:
    enum HandCardRoles {
        CodeRole = Qt::UserRole + 1,
        GlowRole,
        SelectedRole
    };

    CardModel(QObject *parent = 0) : QAbstractListModel(parent) {}

    std::vector<Card>& cards() { return mCards; }
    void addCard(Card &&card);
    void addCards(size_t count);
    Q_INVOKABLE void addCard(QString code);
    Q_INVOKABLE void removeCard(int index);
    Q_INVOKABLE

    void setGlow(int row, bool glow);
    void setSelected(int row, bool selected);
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

protected:
    QHash<int, QByteArray> roleNames() const override;
};