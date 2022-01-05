#pragma once

#include <deque>

#include <QMutex>
#include <QObject>

#include "deckList.h"

class QNetworkAccessManager;
class QNetworkReply;

class ImageLoaderWorker : public QObject
{
    Q_OBJECT
private:
    QNetworkAccessManager *networkManager;
    QMutex mutex;
    std::vector<std::string> cardCodes;
    bool loadInProgress = false;
    const int kTransferTimeout = 15000;

public:
    explicit ImageLoaderWorker();

    void enqueueImageLoad(std::vector<std::string> &&cards);

signals:
    void startLoading();
    void cardImageLoaded();
    void downloadFinished();
    void error(QString message);

private slots:
    void imageDownloadFinished(QNetworkReply *reply);
    void processLoadQueue();

private:
    void imageLoadFailed(QString message);
    void imageLoaded();
    void startNextImageDownload();
};

class ImageLoader : public QObject
{
    Q_OBJECT
private:
    QThread *thread;
    ImageLoaderWorker *worker;
    std::deque<DeckList> decks;
    int currentProgress = 0;

public:
    explicit ImageLoader(QObject *parent = nullptr);
    ~ImageLoader();

    void downloadDeckImages(const DeckList &deck);

signals:
    void markProgress(int percent, QString deckName);
    void deckDownloaded(QString deckName);
    void downloadError(QString message, QString deckName);

private slots:
    void handleError(QString message);
    void onDownloadFinished();
    void onCardImageLoaded();

private:
    void downloadNextDeck();
    int getProgress() const;
};

