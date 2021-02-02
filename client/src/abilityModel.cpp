#pragma once

#include "abilityModel.h"

ActivatedAbility& AbilityModel::activeAbility() {
    for (auto &ab: mAbilities) {
        if (ab.active)
            return ab;
    }
}

void AbilityModel::addAbility(const ActivatedAbility &a) {
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    mAbilities.push_back(a);
    endInsertRows();
}

int AbilityModel::rowCount(const QModelIndex&) const {
    return static_cast<int>(mAbilities.size());
}

QVariant AbilityModel::data(const QModelIndex &index, int role) const {
    if (index.row() < 0 || static_cast<size_t>(index.row()) >= mAbilities.size())
        return QVariant();

    const ActivatedAbility &ab = mAbilities[index.row()];
    switch(role) {
    case CodeRole:
        return QString::fromStdString(ab.code);
    case TextRole:
        return QString::fromStdString(ab.text);
    case ButtonActiveRole:
        return ab.btnActive;
    case ButtonTextRole:
        return ab.btnText;
    case ActiveRole:
        return ab.active;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> AbilityModel::roleNames() const {
    static QHash<int, QByteArray> *roles;
    if (!roles) {
        roles = new QHash<int, QByteArray>;
        (*roles)[CodeRole] = "code";
        (*roles)[TextRole] = "text";
        (*roles)[ButtonActiveRole] = "btnActive";
        (*roles)[ButtonTextRole] = "btnText";
        (*roles)[ActiveRole] = "active";
    }
    return *roles;
}
