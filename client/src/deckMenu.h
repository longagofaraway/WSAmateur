#pragma once

#include <optional>

#include <QQuickItem>

#include "deckMenuModel.h"
#include "imageLoader.h"

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
    ImageLoader *imageLoader;
    std::optional<QNetworkReply*> encoreDecksReply;

public:
    DeckMenu();
    ~DeckMenu() {}

    DeckMenuModel* decksModel() { return &model; }

    Q_INVOKABLE bool addDeck(QString url);
    Q_INVOKABLE void cancelRequest();
    Q_INVOKABLE void downloadImages(int row, int column);

signals:
    void deckDownloadError(QString reason);
    void deckDownloadSuccess();
    void unsupportedCardMet(QStringList unsupportedCards);

private slots:
    void deckDownloaded();
    void loadDecksFromFs();
    void markProgress(int percent, QString deckName);
    void imagesDownloaded(QString deckName);
    void imagesDownloadError(QString message, QString deckName);

private:
    bool deckWithSameNameExists(const DeckList& deck);

protected:
    void componentComplete() override;
};
