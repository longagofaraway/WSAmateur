#pragma once

#include <optional>

#include <QQuickItem>

#include "deckMenuModel.h"

class QNetworkReply;
class QNetworkAccessManager;

class DeckMenu : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(DeckMenuModel *decksModel READ decksModel CONSTANT FINAL)
private:
    DeckMenuModel model;
    QNetworkAccessManager *networkManager;
    std::optional<QNetworkReply*> encoreDecksReply;

public:
    DeckMenu();
    ~DeckMenu() {}

    DeckMenuModel* decksModel() { return &model; }

    Q_INVOKABLE bool addDeck(QString url);
    Q_INVOKABLE void cancelRequest();

signals:
    void deckDownloadError();
    void deckDownloadSuccess();

private slots:
    void deckDownloaded();
    void loadDecksFromFs();

protected:
    void componentComplete() override;
};
