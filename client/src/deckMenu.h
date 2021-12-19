#pragma once

#include <optional>

#include <QQuickItem>

#include "deckMenuModel.h"

class QNetworkReply;
class QNetworkAccessManager;

class DeckList;

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
    void deckDownloadError(QString reason);
    void deckDownloadSuccess();
    void unsupportedCardMet(QStringList unsupportedCards);

private slots:
    void deckDownloaded();
    void loadDecksFromFs();

private:
    bool deckWithSameNameExists(const DeckList& deck);

protected:
    void componentComplete() override;
};
