#pragma once

#include <string>
#include <variant>
#include <vector>

#include <QAbstractListModel>

#include "abilities.pb.h"

#include "abilities.h"

struct ActivatedAbility {
    uint32_t uniqueId;
    ProtoAbilityType type;
    int abilityId;

    int cardId;
    std::string zone;

    std::string code;
    QString text;
    asn::Ability ability;

    bool active = false;
    bool playBtnActive = false;
    bool cancelBtnActive = false;
    QString playBtnText;
    QString cancelBtnText;

    std::variant<std::monostate, asn::ChooseCard, asn::SearchCard> effect;

    ActivatedAbility() : playBtnText("Play") {}
};

class ActivatedAbilityModel : public QAbstractListModel
{
    Q_OBJECT
private:
    std::vector<ActivatedAbility> mAbilities;

public:
    enum AbilityRoles {
        CodeRole = Qt::UserRole + 1,
        TextRole,
        Button1ActiveRole,
        Button2ActiveRole,
        Button1TextRole,
        Button2TextRole,
        ActiveRole
    };
    ActivatedAbilityModel() : QAbstractListModel(nullptr) {}

    int idByUniqueId(uint32_t uniqueId) const;
    int activeId() const;
    void setActive(int row, bool active);
    ActivatedAbility& ability(int index) { return mAbilities.at(index); }
    void addAbility(const ActivatedAbility &a);
    void removeAbility(int row);
    void activatePlayButton(int row, bool active);
    void activateCancelButton(int row, bool active);
    void setPlayBtnText(int row, const QString &text);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int count() const { return rowCount(); }
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

protected:
    QHash<int, QByteArray> roleNames() const override;
};
