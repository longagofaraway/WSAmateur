#include "activatedAbilityModel.h"

#include "player.h"

ActivatedAbilityModel::ActivatedAbilityModel()
    : QAbstractListModel(nullptr) {}

void ActivatedAbilityModel::init(Player *player_) {
    player = player_;
}

void ActivatedAbilityModel::highlightCorrespondingCard(int row, bool on) {
    if (row < 0 || row >= abilities.size())
        return;

    const auto &info = abilities[row];
    auto zone = player->zone(info.zone);
    auto &model = zone->model();

    model.setHighlightedByAbility(model.findById(info.cardId), on);
}

int ActivatedAbilityModel::indexByUniqueId(uint32_t uniqueId) const {
    for (int i = 0; i < static_cast<int>(abilities.size()); ++i) {
        if (abilities[i].uniqueId == uniqueId)
            return i;
    }
    return 0;
}

int ActivatedAbilityModel::activeId() const {
    for (int i = 0; i < static_cast<int>(abilities.size()); ++i) {
        if (abilities[i].active)
            return i;
    }
    return 0;
}

void ActivatedAbilityModel::setActive(int row, bool active) {
    auto index = createIndex(row, 0);
    setData(index, active, ActiveRole);
}

void ActivatedAbilityModel::addAbility(const ActivatedAbility &a) {
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    abilities.push_back(a);
    endInsertRows();
}

void ActivatedAbilityModel::removeAbility(int row) {
    if (static_cast<size_t>(row) >= abilities.size())
        return;

    highlightCorrespondingCard(row, false);

    beginRemoveRows(QModelIndex(), row, row);
    abilities.erase(abilities.begin() + row);
    endRemoveRows();
}

int ActivatedAbilityModel::rowCount(const QModelIndex&) const {
    return static_cast<int>(abilities.size());
}

QVariant ActivatedAbilityModel::data(const QModelIndex &index, int role) const {
    if (static_cast<size_t>(index.row()) >= abilities.size())
        return QVariant();

    const ActivatedAbility &ab = abilities[index.row()];
    switch(role) {
    case CodeRole:
        return QString::fromStdString(ab.code);
    case TextRole:
        return ab.text;
    case Button1ActiveRole:
        return ab.playBtnActive;
    case Button2ActiveRole:
        return ab.cancelBtnActive;
    case Button1TextRole:
        return ab.playBtnText;
    case Button2TextRole:
        return QString("Cancel");
    case ActiveRole:
        return ab.active;
    default:
        return QVariant();
    }
}

void ActivatedAbilityModel::activatePlayButton(int row, bool active) {
    auto index = createIndex(row, 0);
    setData(index, active, Button1ActiveRole);
}

void ActivatedAbilityModel::activateCancelButton(int row, bool active) {
    auto index = createIndex(row, 0);
    setData(index, active, Button2ActiveRole);
}

void ActivatedAbilityModel::setPlayBtnText(int row, const QString &text) {
    auto index = createIndex(row, 0);
    setData(index, text, Button1TextRole);
}

bool ActivatedAbilityModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (static_cast<size_t>(index.row()) >= abilities.size())
        return false;

    ActivatedAbility &ab = abilities[index.row()];

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
    case Button1TextRole:
        ab.playBtnText = value.toString();
        break;
    default:
        return false;
    }

    emit dataChanged(index, index, { role });
    return true;
}

QHash<int, QByteArray> ActivatedAbilityModel::roleNames() const {
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
