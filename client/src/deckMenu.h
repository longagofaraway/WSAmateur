#pragma once

#include <QQuickItem>

#include "deckMenuModel.h"

class DeckMenu : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(DeckMenuModel *decksModel READ decksModel CONSTANT FINAL)
private:
    DeckMenuModel model;

public:
    DeckMenu() {}
    ~DeckMenu() {}

    DeckMenuModel* decksModel() { return &model; }

    Q_INVOKABLE void addDeck(QString url);

protected:
    //void componentComplete() override;
};
