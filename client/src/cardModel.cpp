#include "cardModel.h"

#include <algorithm>


CardModel::CardModel(QObject *parent) : QAbstractListModel(parent) {
    mRoles = QVector<int>() << CodeRole << GlowRole << SelectedRole << TypeRole << StateRole << PowerRole << SoulRole;
}

void CardModel::addCard() {
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    mCards.emplace_back(Card());
    endInsertRows();
    emit countChanged();
}

void CardModel::addCard(QString code) {
    addCard(code.toStdString());
}

void CardModel::addCard(std::string code) {
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    mCards.emplace_back(code);
    endInsertRows();
    emit countChanged();
}

void CardModel::addCards(size_t count) {
    for (size_t i = 0; i < count; ++i)
        addCard();
}

void CardModel::setCard(int row, QString code) {
    if (row < 0 || row >= rowCount())
        return;

    auto modelIndex = index(row);
    mCards[row].init(code.toStdString());
    emit dataChanged(modelIndex, modelIndex, mRoles);
}

void CardModel::swapCards(int from, int to) {
    if (from < 0 ||from >= rowCount()
        || to < 0 || to >= rowCount())
        return;

    auto fromIndex = index(from);
    auto toIndex = index(to);
    std::swap(mCards[from], mCards[to]);
    emit dataChanged(fromIndex, fromIndex, mRoles);
    emit dataChanged(toIndex, toIndex, mRoles);
}

void CardModel::removeCard(int index) {
    if ((size_t)index >= mCards.size())
        return;

    beginRemoveRows(QModelIndex(), index, index);
    mCards.erase(mCards.begin() + index);
    endRemoveRows();
    emit countChanged();
}

void CardModel::setGlow(int row, bool glow) {
    auto index = createIndex(row, 0);
    setData(index, glow, GlowRole);
}

void CardModel::setSelected(int row, bool selected) {
    auto index = createIndex(row, 0);
    setData(index, selected, SelectedRole);
}

void CardModel::setState(int row, CardState state) {
    auto index = createIndex(row, 0);
    setData(index, static_cast<int>(state), StateRole);
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
    case StateRole:
        card.setState(static_cast<CardState>(value.toInt()));
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
    case TypeRole:
        return card.qtype();
    case StateRole:
        return card.qstate();
    case PowerRole:
        return card.power();
    case SoulRole:
        return card.soul();
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
        (*roles)[TypeRole] = "type";
        (*roles)[StateRole] = "state";
        (*roles)[PowerRole] = "power";
        (*roles)[SoulRole] = "soul";
    }
    return *roles;
}

QVector<int> CardModel::mRoles;
