#include "textFrameModel.h"

QString TextFrameModel::text(int row) const {
    if (static_cast<size_t>(row) >= mAbilities.size())
        return "";

    return mAbilities[row].text;
}

void TextFrameModel::addAbility(QString text, bool permanent) {
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    mAbilities.emplace_back(AbilityInfo{ text, permanent });
    endInsertRows();
}

QVariant TextFrameModel::data(const QModelIndex &index, int role) const {
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

QHash<int, QByteArray> TextFrameModel::roleNames() const {
    static QHash<int, QByteArray> *roles;
    if (!roles) {
        roles = new QHash<int, QByteArray>;
        (*roles)[TextRole] = "text";
        (*roles)[PermanentRole] = "permanent";
    }
    return *roles;
}
