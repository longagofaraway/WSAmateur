#pragma once

#include <optional>

#include <QString>

class DeckList;

QString getThumbnail(const DeckList &deck);

bool allImagesDownloaded(const DeckList& deck);

std::optional<DeckList> getDeckByName(QString deckName);
