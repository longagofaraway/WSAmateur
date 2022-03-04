#include "deckMenu.h"

#include <optional>

#include <QDir>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QStandardPaths>

#include <cardDatabase.h>
#include <deckList.h>

#include "filesystemPaths.h"

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

bool allImagesDownloaded(const DeckList& deck) {
    for (const auto &card: deck.cards()) {
        if (!paths::cardImageExists(card.code))
            return false;
    }
    return true;
}
}

DeckMenu::DeckMenu() {
    networkManager = new QNetworkAccessManager(this);
    imageLoader = new ImageLoader(this);
    connect(imageLoader, &ImageLoader::markProgress, this, &DeckMenu::markProgress);
    connect(imageLoader, &ImageLoader::deckDownloaded, this, &DeckMenu::imagesDownloaded);
    connect(imageLoader, &ImageLoader::downloadError, this, &DeckMenu::imagesDownloadError);
}

bool DeckMenu::addDeck(QString url) {
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

void DeckMenu::cancelRequest() {
    if (encoreDecksReply) {
        encoreDecksReply.value()->abort();
        encoreDecksReply.reset();
    }
}

void DeckMenu::downloadImages(int row, int column) {
    model.setDowloadStarted(row, column);

    auto deck = model.getDeck(row, column);
    if (!deck)
        return;
    imageLoader->downloadDeckImages(*deck);
}

void DeckMenu::deckDownloaded() {
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

    if (deckWithSameNameExists(deck)) {
        emit deckDownloadError("Deck with the same name exists");
        return;
    }

    if (!saveDeckToFile(deck)) {
        emit deckDownloadError("Error saving to disk");
        return;
    }


    bool hasAllImages = allImagesDownloaded(deck);
    model.addDeck(deck, hasAllImages, true);
    emit deckDownloadSuccess();
    if (!hasAllImages)
        imageLoader->downloadDeckImages(deck);
}

void DeckMenu::loadDecksFromFs() {
    QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(appData);
    if (!dir.cd("decks")) {
        if (!dir.mkdir("decks"))
            return;
        dir.cd("decks");
    }

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

void DeckMenu::markProgress(int percent, QString deckName) {
    model.updatePercent(deckName, percent);
}

void DeckMenu::imagesDownloaded(QString deckName) {
    model.dowloadFinished(deckName);
}

void DeckMenu::imagesDownloadError(QString message, QString deckName) {
    model.setErrorMessage(deckName, message);
}

bool DeckMenu::deckWithSameNameExists(const DeckList &deck) {
    for (const auto &deckIt: model.items()) {
        if (deckIt.name == deck.qname())
            return true;
    }
    return false;
}

void DeckMenu::componentComplete() {
    QQuickItem::componentComplete();
    loadDecksFromFs();
}
