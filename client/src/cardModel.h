#pragma once

#include <vector>

#include <QAbstractListModel>

#include "cardAttribute.pb.h"

#include "card.h"

class CardZone;

class CardModel : public QAbstractListModel
{
    Q_OBJECT
private:
    std::vector<Card> mCards;

public:
    enum CardRoles {
        CodeRole = Qt::UserRole + 1,
        CardIdRole,
        GlowRole,
        SelectedRole,
        TypeRole,
        StateRole,
        PowerRole,
        SoulRole,
        LevelRole,
        TextModelRole,
        CannotMoveRole,
        HighlightedByAbilityRole,
        TopMarkerRole
    };
    Q_ENUM(CardRoles)
    static QVector<int> mRoles;

    Q_PROPERTY(int count READ count NOTIFY countChanged)

    CardModel(QObject *parent = 0);

    std::vector<Card>& cards() { return mCards; }
    const std::vector<Card>& cards() const { return mCards; }
    void addCards(int count, CardZone *zone);
    void addCard(int id, const std::string &code, CardZone *zone, int targetPos = -1);
    void addCard(CardZone *zone);
    Q_INVOKABLE void setCard(int row, int cardId, QString code);
    Q_INVOKABLE void swapCards(int from, int to);
    Q_INVOKABLE void removeCard(int row);
    Q_INVOKABLE void removeMarker(int row, int markerPos);
    Q_INVOKABLE void addMarker(int row, int cardId, QString code);
    Q_INVOKABLE void clearCard(int row);
    Q_INVOKABLE void clear();
    Q_INVOKABLE int climaxCount();
    Q_INVOKABLE int nonClimaxCount();
    Q_INVOKABLE AbilityModel* textModel(int row);


    void setGlow(int row, bool glow);
    void setSelected(int row, bool selected);
    void setHighlightedByAbility(int row, bool highlighted);
    void setState(int row, asn::State state);
    void setAttr(int row, ProtoCardAttribute attr, int value);
    void setCannotMove(int row, int value);
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int count() const { return rowCount(); }
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    int findById(int id) const;

signals:
    void countChanged();

protected:
    QHash<int, QByteArray> roleNames() const override;
};
