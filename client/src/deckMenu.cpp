#include "deckMenu.h"

#include <optional>

#include <QDir>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QStandardPaths>

#include <deckList.h>

namespace {
QString getThumbnail(const DeckList &deck) {
    auto& cards = deck.cards();
    if (cards.empty())
        return "cardback";

    return QString::fromStdString(cards.front().code);
}

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
}

DeckMenu::DeckMenu() {
    networkManager = new QNetworkAccessManager(this);
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

void DeckMenu::deckDownloaded() {
    encoreDecksReply.reset();
    auto *reply = dynamic_cast<QNetworkReply*>(sender());
    QNetworkReply::NetworkError errorCode = reply->error();
    if (errorCode != QNetworkReply::NoError) {
        reply->deleteLater();
        emit deckDownloadError();
        return;
    }

    QByteArray data = reply->readAll();
    reply->deleteLater();

    DeckList deck;
    if (!deck.fromEncoreDecksResponse(data)) {
        emit deckDownloadError();
        return;
    }

    if (!saveDeckToFile(deck)) {
        emit deckDownloadError();
        return;
    }

    model.addDeck(DeckMenuItem{QString::fromStdString(deck.name()), getThumbnail(deck)});
    emit deckDownloadSuccess();
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
        items.emplace_back(DeckMenuItem{
                               QString::fromStdString(deckList.name()),
                               getThumbnail(deckList)
                           });
    }

    model.setDecks(std::move(items));
}

void DeckMenu::componentComplete() {
    QQuickItem::componentComplete();
    loadDecksFromFs();
}
