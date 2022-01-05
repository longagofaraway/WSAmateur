#include "decksListWindow.h"

#include <optional>

#include <QDir>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QStandardPaths>

#include <cardDatabase.h>
#include <deckList.h>

#include "filesystemPaths.h"
#include "deckUtils.h"

namespace {
bool saveDeckToFile(const DeckList &deck) {
    QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(appData);
    dir.cd("decks");

    if (dir.exists(deck.qname()))
        return false;

    QFile file(dir.filePath(deck.qname()) + ".cod");
    if (!file.open(QIODevice::WriteOnly))
        return false;

    QTextStream stream(&file);
    stream << deck.toXml();
    file.close();

    return true;
}

QStringList checkDeckCardsSupported(const DeckList& deck) {
    const auto &cardDb = CardDatabase::get();
    QStringList unsupportedCards;
    for (const auto &card: deck.cards()) {
        auto cardInfo = cardDb.getCard(card.code);
        if (cardInfo)
            continue;
        unsupportedCards.append(QString::fromStdString(card.code));
    }
    return unsupportedCards;
}
}

DecksListWindow::DecksListWindow() {
    networkManager = new QNetworkAccessManager(this);
    imageLoader = new ImageLoader(this);
    connect(imageLoader, &ImageLoader::markProgress, this, &DecksListWindow::markProgress);
    connect(imageLoader, &ImageLoader::deckDownloaded, this, &DecksListWindow::imagesDownloaded);
    connect(imageLoader, &ImageLoader::downloadError, this, &DecksListWindow::imagesDownloadError);
}

bool DecksListWindow::addDeckFromEncoreDecks(QString url) {
    int pos = url.lastIndexOf("/");
    if (pos == -1)
        return false;

    QString deckId = url.mid(pos + 1);
    QString encoreDecksDeckApiUrl = "https://www.encoredecks.com/api/deck/" + deckId;
    QUrl deckUrl = QUrl::fromUserInput(encoreDecksDeckApiUrl);
    if (!deckUrl.isValid())
        return false;

    encoreDecksReply = networkManager->get(QNetworkRequest(deckUrl));

    connect(*encoreDecksReply, SIGNAL(finished()), this, SLOT(deckDownloaded()));

    return true;
}

bool DecksListWindow::addDeckToModel(QString xmlDeck) {
    DeckList deck;
    if (!deck.fromXml(xmlDeck)) {
        emit deckDownloadError("Parsing error");
        return false;
    }

    bool hasAllImages = allImagesDownloaded(deck);
    model.addDeck(deck, hasAllImages, true);
    if (!hasAllImages)
        imageLoader->downloadDeckImages(deck);
    else
        emit deckImagesDownloaded();

    return true;
}

void DecksListWindow::cancelRequest() {
    if (encoreDecksReply) {
        encoreDecksReply.value()->abort();
        encoreDecksReply.reset();
    }
}

void DecksListWindow::downloadImages(int row, int column) {
    model.setDowloadStarted(row, column);

    auto deck = model.getDeck(row, column);
    if (!deck)
        return;
    imageLoader->downloadDeckImages(*deck);
}

void DecksListWindow::deckDownloaded() {
    encoreDecksReply.reset();
    auto *reply = dynamic_cast<QNetworkReply*>(sender());
    QNetworkReply::NetworkError errorCode = reply->error();
    if (errorCode != QNetworkReply::NoError) {
        reply->deleteLater();
        emit deckDownloadError("Network error");
        return;
    }

    QByteArray data = reply->readAll();
    reply->deleteLater();

    DeckList deck;
    if (!deck.fromEncoreDecksResponse(data)) {
        emit deckDownloadError("Parsing error");
        return;
    }

    auto unsupportedCards = checkDeckCardsSupported(deck);
    if (!unsupportedCards.empty()) {
        emit unsupportedCardMet(unsupportedCards);
        return;
    }

    if (!saveDeckToFile(deck)) {
        emit deckDownloadError("Error saving to disk");
        return;
    }

    if (deckWithSameNameExists(deck)) {
        emit deckDownloadError("Deck with the same name exists");
        return;
    }

    bool hasAllImages = allImagesDownloaded(deck);
    model.addDeck(deck, hasAllImages, true);
    emit deckDownloadSuccess();
    if (!hasAllImages)
        imageLoader->downloadDeckImages(deck);
}

void DecksListWindow::loadDecksFromFs() {
    QDir dir = paths::decksDir();
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
    QStringList fileList = dir.entryList();

    std::vector<DeckMenuItem> items;
    for (const auto &fileName: fileList) {
        QFile file(dir.filePath(fileName));
        if (!file.open(QIODevice::ReadOnly))
            continue;

        QTextStream deckFile(&file);
        auto deck = deckFile.readAll();
        DeckList deckList;
        if (!deckList.fromXml(deck))
            continue;

        auto unsupportedCards = checkDeckCardsSupported(deckList);
        if (!unsupportedCards.empty())
            continue;

        items.emplace_back(deckList, allImagesDownloaded(deckList));
    }

    model.setDecks(std::move(items));
}

void DecksListWindow::markProgress(int percent, QString deckName) {
    model.updatePercent(deckName, percent);
}

void DecksListWindow::imagesDownloaded(QString deckName) {
    model.dowloadFinished(deckName);
    emit deckImagesDownloaded();
}

void DecksListWindow::imagesDownloadError(QString message, QString deckName) {
    model.setErrorMessage(deckName, message);
}

bool DecksListWindow::deckWithSameNameExists(const DeckList &deck) {
    for (const auto &deckIt: model.items()) {
        if (deckIt.name == deck.qname())
            return true;
    }
    return false;
}

void DecksListWindow::componentComplete() {
    QQuickItem::componentComplete();
    bool isLocal = property("local").toBool();
    if (isLocal)
        loadDecksFromFs();
    else
        setProperty("visible", false);
}
