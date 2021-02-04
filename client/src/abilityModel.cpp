#pragma once

#include "abilityModel.h"

int AbilityModel::idByUniqueId(uint32_t uniqueId) const {
    for (int i = 0; i < static_cast<int>(mAbilities.size()); ++i) {
        if (mAbilities[i].uniqueId == uniqueId)
            return i;
    }
    return 0;
}

int AbilityModel::activeId() const {
    for (int i = 0; i < static_cast<int>(mAbilities.size()); ++i) {
        if (mAbilities[i].active)
            return i;
    }
    return 0;
}

void AbilityModel::setActive(int row, bool active) {
    auto index = createIndex(row, 0);
    setData(index, active, ActiveRole);
}

void AbilityModel::addAbility(const ActivatedAbility &a) {
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    mAbilities.push_back(a);
    endInsertRows();
}

void AbilityModel::removeAbility(int row) {
    if (static_cast<size_t>(row) >= mAbilities.size())
        return;

    beginRemoveRows(QModelIndex(), row, row);
    mAbilities.erase(mAbilities.begin() + row);
    endRemoveRows();
}

int AbilityModel::rowCount(const QModelIndex&) const {
    return static_cast<int>(mAbilities.size());
}

QVariant AbilityModel::data(const QModelIndex &index, int role) const {
    if (static_cast<size_t>(index.row()) >= mAbilities.size())
        return QVariant();

    const ActivatedAbility &ab = mAbilities[index.row()];
    switch(role) {
    case CodeRole:
        return QString::fromStdString(ab.code);
    case TextRole:
        return QString::fromStdString(ab.text);
    case Button1ActiveRole:
        return ab.playBtnActive;
    case Button2ActiveRole:
        return ab.cancelBtnActive;
    case Button1TextRole:
        return QString("Play");
    case Button2TextRole:
        return QString("Cancel");
    case ActiveRole:
        return ab.active;
    default:
        return QVariant();
    }
}

void AbilityModel::activatePlayButton(int row, bool active) {
    auto index = createIndex(row, 0);
    setData(index, active, Button1ActiveRole);
}

void AbilityModel::activateCancelButton(int row, bool active) {
    auto index = createIndex(row, 0);
    setData(index, active, Button2ActiveRole);
}

bool AbilityModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (static_cast<size_t>(index.row()) >= mAbilities.size())
        return false;

    ActivatedAbility &ab = mAbilities[index.row()];

    switch(role) {
    case ActiveRole:
        ab.active = value.toBool();
        break;
    case Button1ActiveRole:
        ab.playBtnActive = value.toBool();
        break;
    case Button2ActiveRole:
        ab.cancelBtnActive = value.toBool();
        break;
    default:
        return false;
    }

    emit dataChanged(index, index, { role });
    return true;
}

QHash<int, QByteArray> AbilityModel::roleNames() const {
    static QHash<int, QByteArray> *roles;
    if (!roles) {
        roles = new QHash<int, QByteArray>;
        (*roles)[CodeRole] = "code";
        (*roles)[TextRole] = "text";
        (*roles)[Button1ActiveRole] = "playBtnActive";
        (*roles)[Button2ActiveRole] = "cancelBtnActive";
        (*roles)[Button1TextRole] = "playBtnText";
        (*roles)[Button2TextRole] = "cancelBtnText";
        (*roles)[ActiveRole] = "active";
    }
    return *roles;
}
