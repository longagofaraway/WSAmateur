#include "abilityModel.h"

#include <stdexcept>

QString AbilityModel::text(int row) const {
    if (static_cast<size_t>(row) >= mAbilities.size())
        return "";

    return mAbilities[row].text;
}

const asn::Ability &AbilityModel::ability(int row) const {
    if (static_cast<size_t>(row) >= mAbilities.size())
        throw std::runtime_error("wrong ability id");

    return mAbilities[row].ability;
}

void AbilityModel::addAbility(const asn::Ability &a, asn::CardType cardType,  bool permanent) {
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    mAbilities.emplace_back(QString::fromStdString(printAbility(a, cardType)), a, permanent);
    endInsertRows();
}

void AbilityModel::removeAbility(int row) {
    if (static_cast<size_t>(row) >= mAbilities.size())
        return;

    beginRemoveRows(QModelIndex(), row, row);
    mAbilities.erase(mAbilities.begin() + row);
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
