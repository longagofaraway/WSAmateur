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
        SelectedRole,
        TypeRole,
        StateRole,
        PowerRole,
        SoulRole
    };
    Q_ENUM(HandCardRoles)
    static QVector<int> mRoles;

    Q_PROPERTY(int count READ count NOTIFY countChanged)

    CardModel(QObject *parent = 0);

    std::vector<Card>& cards() { return mCards; }
    void addCards(int count);
    void addCard(std::string code);
    Q_INVOKABLE void addCard();
    Q_INVOKABLE void addCard(QString code);
    Q_INVOKABLE void setCard(int row, QString code);
    Q_INVOKABLE void swapCards(int from, int to);
    Q_INVOKABLE void removeCard(int index);

    void setGlow(int row, bool glow);
    void setSelected(int row, bool selected);
    void setState(int row, CardState state);
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int count() const { return rowCount(); }
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

signals:
    void countChanged();

protected:
    QHash<int, QByteArray> roleNames() const override;
};
