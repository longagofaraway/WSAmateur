#pragma once

#include <vector>

#include <QAbstractListModel>

struct AbilityInfo {
    QString text;
    bool permanent = true;
};

class TextFrameModel : public QAbstractListModel
{
    Q_OBJECT
private:
    std::vector<AbilityInfo> mAbilities;

public:
    enum AbilityRoles {
        TextRole = Qt::UserRole + 1,
        PermanentRole
    };
    TextFrameModel() : QAbstractListModel(nullptr) {}

    QString text(int row) const;
    void addAbility(QString text, bool permanent = true);
    int rowCount(const QModelIndex & = QModelIndex()) const override { return static_cast<int>(mAbilities.size()); }
    int count() const { return rowCount(); }
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

protected:
    QHash<int, QByteArray> roleNames() const override;
};
