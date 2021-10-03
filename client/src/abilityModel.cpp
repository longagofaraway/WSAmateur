#include "abilityModel.h"

#include <stdexcept>

QString AbilityModel::textByIndex(int row) const {
    if (static_cast<size_t>(row) >= abilities.size())
        return "";

    return abilities[row].text;
}

QStringList AbilityModel::getReferences() {
    return references;
}

void AbilityModel::setReferences(std::vector<std::string> refs) {
    references.clear();
    for (const auto &ref: refs) {
        references.push_back(QString::fromStdString(ref));
    }
}

QString AbilityModel::textById(int id) const {
    auto it = std::find_if(abilities.begin(), abilities.end(), [id](const AbilityInfo &i) { return id == i.id; });
    if (it == abilities.end())
        throw std::runtime_error("wrong ability id");

    return it->text;
}

const asn::Ability &AbilityModel::ability(int row) const {
    if (static_cast<size_t>(row) >= abilities.size())
        throw std::runtime_error("wrong ability id");

    return abilities[row].ability;
}

const asn::Ability &AbilityModel::abilityById(int id) const {
    auto it = std::find_if(abilities.begin(), abilities.end(), [id](const AbilityInfo &i) { return id == i.id; });
    if (it == abilities.end())
        throw std::runtime_error("wrong ability id");

    return it->ability;
}

void AbilityModel::addAbility(const asn::Ability &a, int id, asn::CardType cardType, bool permanent) {
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    abilities.emplace_back(QString::fromStdString(printAbility(a, cardType)), a, id, permanent);
    endInsertRows();
}

void AbilityModel::removeAbilityById(int id) {
    auto it = std::find_if(abilities.begin(), abilities.end(), [id](const AbilityInfo &i) { return id == i.id; });
    if (it == abilities.end())
        return;
    int row = it - abilities.begin();

    beginRemoveRows(QModelIndex(), row, row);
    abilities.erase(it);
    endRemoveRows();
}

QVariant AbilityModel::data(const QModelIndex &index, int role) const {
    if (static_cast<size_t>(index.row()) >= abilities.size())
        return QVariant();

    switch(role) {
    case TextRole:
        return abilities[index.row()].text;
    case PermanentRole:
        return abilities[index.row()].permanent;
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
