#pragma once

#include <vector>

#include <QAbstractListModel>
#include <QString>

class Game;
class Player;

class QQuickItem;

class ChoiceDialogModel : public QAbstractListModel
{
    Q_OBJECT
private:
    std::vector<QString> mData;

public:
    enum Roles {
        TextRole = Qt::UserRole + 1
    };

    ChoiceDialogModel() : QAbstractListModel(nullptr) {}

    void clear();
    void addRow(const QString &data);
    void setChoice(const std::vector<QString> &data);
    int rowCount(const QModelIndex & = QModelIndex()) const override { return static_cast<int>(mData.size()); };
    int count() const { return rowCount(); }
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

protected:
    QHash<int, QByteArray> roleNames() const override;
};


class ChoiceDialogBase : public QObject {
    Q_OBJECT
protected:
    ChoiceDialogModel mModel;
    QQuickItem *mQmlObject;
    Player *mPlayer;

public:
    ChoiceDialogBase(Game *game);
    ~ChoiceDialogBase() noexcept;

public slots:
    void destroy();
};


class ChoiceDialog : public ChoiceDialogBase {
    Q_OBJECT
public:
    ChoiceDialog(Game *game);
    ~ChoiceDialog() noexcept;

    void setData(const QString &header, const std::vector<QString> &data);

public slots:
    void processChoice(int choice);
};


class ActChoiceDialog : public ChoiceDialogBase {
    Q_OBJECT
private:
    struct AbilityData {
        int abilityId;
        int cardPos;
    };

    std::vector<AbilityData> mAbilities;

public:
    ActChoiceDialog(Game *game);

    void addAbility(int cardPos, int abilityId, const QString &text);
    void show();

public slots:
    void processChoice(int choice);
};
