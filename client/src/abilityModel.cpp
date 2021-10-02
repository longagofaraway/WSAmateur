#include "abilityModel.h"

#include <stdexcept>

QString AbilityModel::textByIndex(int row) const {
    if (static_cast<size_t>(row) >= mAbilities.size())
        return "";

    return mAbilities[row].text;
}

QString AbilityModel::textById(int id) const {
    auto it = std::find_if(mAbilities.begin(), mAbilities.end(), [id](const AbilityInfo &i) { return id == i.id; });
    if (it == mAbilities.end())
        throw std::runtime_error("wrong ability id");

    return it->text;
}

const asn::Ability &AbilityModel::ability(int row) const {
    if (static_cast<size_t>(row) >= mAbilities.size())
        throw std::runtime_error("wrong ability id");

    return mAbilities[row].ability;
}

const asn::Ability &AbilityModel::abilityById(int id) const {
    auto it = std::find_if(mAbilities.begin(), mAbilities.end(), [id](const AbilityInfo &i) { return id == i.id; });
    if (it == mAbilities.end())
        throw std::runtime_error("wrong ability id");

    return it->ability;
}

void AbilityModel::addAbility(const asn::Ability &a, int id, asn::CardType cardType, bool permanent) {
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    mAbilities.emplace_back(QString::fromStdString(printAbility(a, cardType)), a, id, permanent);
    endInsertRows();
}

void AbilityModel::removeAbilityById(int id) {
    auto it = std::find_if(mAbilities.begin(), mAbilities.end(), [id](const AbilityInfo &i) { return id == i.id; });
    if (it == mAbilities.end())
        return;
    int row = it - mAbilities.begin();

    beginRemoveRows(QModelIndex(), row, row);
    mAbilities.erase(it);
    endRemoveRows();
}

QVariant AbilityModel::data(const QModelIndex &index, int role) const {
    if (static_cast<size_t>(index.row()) >= mAbilities.size())
        return QVariant();

    switch(role) {
    case TextRole:
        return mAbilities[index.row()].text;
    case PermanentRole:
        return mAbilities[index.row()].permanent;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> AbilityModel::roleNames() const {
    static QHash<int, QByteArray> *roles;
    if (!roles) {
        roles = new QHash<int, QByteArray>;
        (*roles)[TextRole] = "text";
        (*roles)[PermanentRole] = "permanent";
    }
    return *roles;
}
