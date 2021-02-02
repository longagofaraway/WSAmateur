#pragma once

#include <string>
#include <vector>

#include <QAbstractListModel>

#include "abilities.pb.h"

struct ActivatedAbility {
    ProtoAbilityType type;
    int id;

    int cardId;
    std::string zone;

    std::string code;
    std::string text;

    bool active = false;
    bool btnActive = false;
    QString btnText;
};

class AbilityModel : public QAbstractListModel
{
    Q_OBJECT
private:
    std::vector<ActivatedAbility> mAbilities;

public:
    enum AbilityRoles {
        CodeRole = Qt::UserRole + 1,
        TextRole,
        ButtonActiveRole,
        ButtonTextRole,
        ActiveRole
    };
    AbilityModel() : QAbstractListModel(nullptr) {}

    ActivatedAbility& activeAbility();
    void addAbility(const ActivatedAbility &a);

    //bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int count() const { return rowCount(); }
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

protected:
    QHash<int, QByteArray> roleNames() const override;
};
