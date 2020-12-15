#include "cardModel.h"


void CardModel::addCard(Card &&card) {
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    mCards.emplace_back(std::move(card));
    endInsertRows();
}

void CardModel::addCard() {
    addCard(Card());
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

void CardModel::setGlow(int row, bool glow) {
    auto index = createIndex(row, 0);
    setData(index, glow, GlowRole);
}

void CardModel::setSelected(int row, bool selected) {
    auto index = createIndex(row, 0);
    setData(index, selected, SelectedRole);
}

bool CardModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (index.row() < 0 || static_cast<size_t>(index.row()) >= mCards.size())
        return false;

    Card &card = mCards[index.row()];

    switch(role) {
    case GlowRole:
        card.setGlow(value.toBool());
        break;
    case SelectedRole:
        card.setSelected(value.toBool());
        break;
    default:
        return false;
    }

    emit dataChanged(index, index, { role });
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
    case SelectedRole:
        return card.selected();
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
        (*roles)[SelectedRole] = "selected";
    }
    return *roles;
}
