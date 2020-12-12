#include "cardModel.h"


void CardModel::addCard(Card &&card) {
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    mCards.emplace_back(std::move(card));
    endInsertRows();
}


void CardModel::addCard(QString code) {
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    mCards.emplace_back(code.toStdString());
    endInsertRows();
}

void CardModel::addCards(size_t count) {
    mCards = std::vector<Card>(count, { "" });
}

void CardModel::removeCard(int index) {
    if ((size_t)index >= mCards.size())
        return;

    beginRemoveRows(QModelIndex(), index, index);
    mCards.erase(mCards.begin() + index);
    endRemoveRows();
}

bool CardModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (index.row() < 0 || static_cast<size_t>(index.row()) >= mCards.size())
        return false;

    if (role != GlowRole)
        return false;

    Card &card = mCards[index.row()];

    card.setGlow(value.toBool());
    emit dataChanged(index, index, { GlowRole });
    return true;
}

int CardModel::rowCount(const QModelIndex &) const {
    return static_cast<int>(mCards.size());
}

QVariant CardModel::data(const QModelIndex &index, int role) const {
    if (index.row() < 0 || static_cast<size_t>(index.row()) >= mCards.size())
        return QVariant();

    const Card &card = mCards[index.row()];
    switch(role) {
    case CodeRole:
        return card.qcode();
    case GlowRole:
        return card.glow();
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> CardModel::roleNames() const {
    static QHash<int, QByteArray> *roles;
    if (!roles) {
        roles = new QHash<int, QByteArray>;
        (*roles)[CodeRole] = "code";
        (*roles)[GlowRole] = "glow";
    }
    return *roles;
}
