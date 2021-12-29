#include "imageLoader.h"

#include <QFile>
#include <QImage>
#include <QImageReader>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QThread>
#include <QUrl>

#include "filesystemPaths.h"
#include "imageLinks.h"

namespace {
std::vector<std::string> getCardsToDownload(const DeckList &deck) {
    std::vector<std::string> queueCards;
    for (const auto &card: deck.cards()) {
        if (paths::cardImageExists(card.code))
            continue;

        queueCards.push_back(card.code);
    }
    return queueCards;
}
}

ImageLoader::ImageLoader(QObject *parent)
    : QObject(parent) {
    worker = new ImageLoaderWorker();
    connect(worker, &ImageLoaderWorker::error, this, &ImageLoader::handleError, Qt::QueuedConnection);
    connect(worker, &ImageLoaderWorker::downloadFinished, this, &ImageLoader::onDownloadFinished, Qt::QueuedConnection);
    connect(worker, &ImageLoaderWorker::cardImageLoaded, this, &ImageLoader::onCardImageLoaded, Qt::QueuedConnection);
}

ImageLoader::~ImageLoader() {
    worker->deleteLater();
}

void ImageLoader::downloadDeckImages(const DeckList &deck) {
    auto queueCards = getCardsToDownload(deck);

    if (queueCards.empty()) {
        emit deckDownloaded(deck.qname());
        return;
    }
    decks.push_back(deck);

    if (decks.size() == 1)
        downloadNextDeck();
}

void ImageLoader::handleError(QString message) {
    const auto &deck = decks.front();
    emit downloadError(message, deck.qname());
    decks.pop_front();
    downloadNextDeck();
}

void ImageLoader::onDownloadFinished() {
    if (!decks.empty()) {
        const auto &deck = decks.front();
        emit deckDownloaded(deck.qname());
        decks.pop_front();
    }
    downloadNextDeck();
}

void ImageLoader::onCardImageLoaded() {
    const auto &deck = decks.front();
    currentProgress++;
    emit markProgress(getProgress(), deck.qname());
}

void ImageLoader::downloadNextDeck() {
    if (decks.empty())
        return;
    const auto &deck = decks.front();
    auto queueCards = getCardsToDownload(deck);
    currentProgress = static_cast<int>(deck.cards().size() - queueCards.size());
    emit markProgress(getProgress(), deck.qname());
    worker->enqueueImageLoad(std::move(queueCards));
}

int ImageLoader::getProgress() const {
    const auto &deck = decks.front();
    return 100 * currentProgress / static_cast<int>(deck.cards().size());
}

void ImageLoaderWorker::imageDownloadFinished(QNetworkReply *reply) {
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode == 301 || statusCode == 302) {
        QUrl redirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        QNetworkRequest req(redirectUrl);
        networkManager->get(req);
        return;
    }

    const auto &picData = reply->peek(reply->size());

    QImage testImage;

    QImageReader imgReader;
    imgReader.setDecideFormatFromContent(true);
    imgReader.setDevice(reply);
    QString extension = "." + imgReader.format();
    // the format is determined prior to reading the QImageReader data
    // into a QImage object, as that wipes the QImageReader buffer
    if (extension == ".jpeg") {
        extension = ".jpg";
    }

    if (imgReader.read(&testImage)) {
        auto cardToDownload = cardCodes.back();
        QFile newImage(paths::cardImagePath(cardToDownload) + extension);
        if (!newImage.open(QIODevice::WriteOnly)) {
            reply->deleteLater();
            return;
        }
        newImage.write(picData);
        newImage.close();
        imageLoaded();
    } else {
        imageLoadFailed("Network error");
    }

    reply->deleteLater();
    startNextImageDownload();
}

void ImageLoaderWorker::processLoadQueue() {
    if (loadInProgress)
        return;

    startNextImageDownload();
}

void ImageLoaderWorker::imageLoadFailed(QString message) {
    emit error(message);
    loadInProgress = false;
    QMutexLocker locker(&mutex);
    cardCodes.clear();
}

void ImageLoaderWorker::imageLoaded() {
    emit cardImageLoaded();
    QMutexLocker locker(&mutex);
    cardCodes.pop_back();
}

void ImageLoaderWorker::startNextImageDownload() {
    QMutexLocker locker(&mutex);
    if (cardCodes.empty()) {
        loadInProgress = false;
        emit downloadFinished();
        return;
    }

    auto cardToDownload = cardCodes.back();
    auto imageUrl = ImageLinks::get().imageLink(cardToDownload);
    locker.unlock();
    if (!imageUrl) {
        imageLoadFailed("No image url found");
        return;
    }

    loadInProgress = true;

    QUrl url(imageUrl.value());
    QNetworkRequest req(url);
    networkManager->get(req);
}

ImageLoaderWorker::ImageLoaderWorker() {
    connect(this, SIGNAL(startLoading()), this, SLOT(processLoadQueue()), Qt::QueuedConnection);

    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(imageDownloadFinished(QNetworkReply*)));

    thread = new QThread;
    thread->start();
    moveToThread(thread);
}

void ImageLoaderWorker::enqueueImageLoad(std::vector<std::string> &&cards) {
    QMutexLocker locker(&mutex);

    cardCodes.insert(cardCodes.end(), std::make_move_iterator(cards.begin()),
                                      std::make_move_iterator(cards.end()));

    locker.unlock();
    emit startLoading();
}
