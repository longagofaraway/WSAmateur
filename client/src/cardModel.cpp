#include "cardModel.h"

#include <algorithm>


CardModel::CardModel(QObject *parent) : QAbstractListModel(parent) {
    mRoles = QVector<int>() << CodeRole << CardIdRole << GlowRole << SelectedRole << TypeRole
                            << StateRole << PowerRole << SoulRole << LevelRole;
}

void CardModel::clear() {
    if (!mCards.size())
        return;

    beginRemoveRows(QModelIndex(), 0, rowCount() - 1);
    mCards.clear();
    endRemoveRows();
    emit countChanged();
}

void CardModel::addCard(CardZone *zone) {
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    mCards.emplace_back(Card(zone));
    endInsertRows();
    emit countChanged();
}

void CardModel::addCard(int id, const std::string &code, CardZone *zone, int targetPos) {
    int row = targetPos;
    if (targetPos == -1)
        row = rowCount();
    beginInsertRows(QModelIndex(), row, row);
    if (targetPos == -1)
        mCards.emplace_back(id, code, zone);
    else
        mCards.emplace(mCards.begin() + row, id, code, zone);
    endInsertRows();
    emit countChanged();
}

void CardModel::addCards(int count, CardZone *zone) {
    for (int i = 0; i < count; ++i)
        addCard(zone);
}

void CardModel::setCard(int row, int cardId, QString code) {
    if (row < 0 || row >= rowCount())
        return;

    auto modelIndex = index(row);
    mCards[row].init(cardId, code.toStdString());
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

void CardModel::removeCard(int row) {
    if (static_cast<size_t>(row) >= mCards.size())
        return;

    beginRemoveRows(QModelIndex(), row, row);
    mCards.erase(mCards.begin() + row);
    endRemoveRows();
    emit countChanged();
}

void CardModel::clearCard(int row) {
    if (static_cast<size_t>(row) >= mCards.size())
        return;

    auto modelIndex = index(row);
    mCards[row].clear();
    emit dataChanged(modelIndex, modelIndex, mRoles);
}

int CardModel::climaxCount() {
    int count = 0;
    for (const auto &card: mCards)
        if (card.type() == CardType::Climax)
            ++count;
    return count;
}

int CardModel::nonClimaxCount() {
    int count = 0;
    for (const auto &card: mCards)
        if (card.type() != CardType::Climax)
            ++count;
    return count;
}

AbilityModel *CardModel::textModel(int row) {
    if (static_cast<size_t>(row) >= mCards.size())
        return nullptr;
    return mCards[row].textModel();
}

void CardModel::setGlow(int row, bool glow) {
    auto index = createIndex(row, 0);
    setData(index, glow, GlowRole);
}

void CardModel::setSelected(int row, bool selected) {
    auto index = createIndex(row, 0);
    setData(index, selected, SelectedRole);
}

void CardModel::setState(int row, asn::State state) {
    auto index = createIndex(row, 0);
    setData(index, static_cast<int>(state), StateRole);
}

void CardModel::setAttr(int row, ProtoCardAttribute attr, int value) {
    auto index = createIndex(row, 0);
    int role;
    if (attr == ProtoAttrSoul)
        role = SoulRole;
    else if (attr == ProtoAttrPower)
        role = PowerRole;
    else
        role = LevelRole;
    setData(index, value, role);
}

bool CardModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (static_cast<size_t>(index.row()) >= mCards.size())
        return false;

    Card &card = mCards[index.row()];
    if (!card.cardPresent() && role != GlowRole)
        return false;

    switch(role) {
    case GlowRole:
        card.setGlow(value.toBool());
        break;
    case SelectedRole:
        card.setSelected(value.toBool());
        break;
    case StateRole:
        card.setState(static_cast<asn::State>(value.toInt()));
        break;
    case SoulRole:
        card.setSoul(value.toInt());
        break;
    case PowerRole:
        card.setPower(value.toInt());
        break;
    case LevelRole:
        card.setLevel(value.toInt());
        break;
    default:
        return false;
    }

    emit dataChanged(index, index, { role });
    return true;
}

int CardModel::rowCount(const QModelIndex&) const {
    return static_cast<int>(mCards.size());
}

QVariant CardModel::data(const QModelIndex &index, int role) const {
    if (static_cast<size_t>(index.row()) >= mCards.size())
        return QVariant();

    const Card &card = mCards[index.row()];
    switch(role) {
    case CodeRole:
        return card.qcode();
    case CardIdRole:
        return card.id();
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
    case LevelRole:
        return card.level();
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> CardModel::roleNames() const {
    static QHash<int, QByteArray> *roles;
    if (!roles) {
        roles = new QHash<int, QByteArray>;
        (*roles)[CodeRole] = "code";
        (*roles)[CardIdRole] = "cardId";
        (*roles)[GlowRole] = "glow";
        (*roles)[SelectedRole] = "selected";
        (*roles)[TypeRole] = "type";
        (*roles)[StateRole] = "state";
        (*roles)[PowerRole] = "power";
        (*roles)[SoulRole] = "soul";
        (*roles)[LevelRole] = "level";
    }
    return *roles;
}

QVector<int> CardModel::mRoles;
