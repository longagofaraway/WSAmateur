#include "deckUtils.h"

#include <QTextStream>

#include "deckList.h"
#include "filesystemPaths.h"

QString getThumbnail(const DeckList &deck) {
    auto& cards = deck.cards();
    if (cards.empty())
        return "cardback";

    return QString::fromStdString(cards.front().code);
}

bool allImagesDownloaded(const DeckList& deck) {
    for (const auto &card: deck.cards()) {
        if (!paths::cardImageExists(card.code))
            return false;
    }
    return true;
}

std::optional<DeckList> getDeckByName(QString deckName) {
    QDir dir = paths::decksDir();
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
    QStringList fileList = dir.entryList();

    for (const auto &fileName: fileList) {
        QFile file(dir.filePath(fileName));
        if (!file.open(QIODevice::ReadOnly))
            continue;

        QTextStream deckFile(&file);
        auto deck = deckFile.readAll();
        DeckList deckList;
        if (!deckList.fromXml(deck))
            continue;
        if (deckList.qname() == deckName)
            return deckList;
    }

    return {};
}
