#include "deckMenuModel.h"


DeckMenuModel::DeckMenuModel() {
    decks.push_back(DeckMenuItem{"deck1"});
    decks.push_back(DeckMenuItem{"deck2"});
    decks.push_back(DeckMenuItem{"deck3"});
    decks.push_back(DeckMenuItem{"deck4"});
    decks.push_back(DeckMenuItem{"last element"});
}

void DeckMenuModel::setDecks(std::vector<DeckMenuItem> &&newData) {
    newData.push_back(DeckMenuItem{"last element"});
    beginResetModel();
    decks.swap(newData);
    endResetModel();
}

void DeckMenuModel::addDeck(DeckMenuItem &&deck) {
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    if (decks.empty())
        decks.emplace_back(std::move(deck));
    else
        decks.emplace(std::prev(decks.end()), std::move(deck));
    endInsertRows();
}

int DeckMenuModel::rowCount(const QModelIndex &) const {
    return static_cast<int>(decks.size() / kColumnCount) + 1;
}

int DeckMenuModel::columnCount(const QModelIndex &) const {
    return kColumnCount;
}

QVariant DeckMenuModel::data(const QModelIndex &index, int role) const {
    int vectorIndex = index.row() * kColumnCount + index.column();
    bool indexValid = true;
    if (static_cast<size_t>(vectorIndex) >= decks.size())
        indexValid = false;

    const auto &deck = decks[vectorIndex];
    switch (role) {
    case NameRole:
        return indexValid ? decks[vectorIndex].name : "";
    case ThumbnailRole:
        return indexValid ? decks[vectorIndex].thumbnail : "cardback";
    case LastElementRole:
        return vectorIndex == decks.size() - 1;
    case InvalidElementRole:
        return !indexValid;
    }
    return QVariant{};
}

QHash<int, QByteArray> DeckMenuModel::roleNames() const {
    static QHash<int, QByteArray> *roles;
    if (!roles) {
        roles = new QHash<int, QByteArray>;
        (*roles)[NameRole] = "deckName";
        (*roles)[ThumbnailRole] = "thumbnail";
        (*roles)[LastElementRole] = "lastElement";
        (*roles)[InvalidElementRole] = "invalidElement";
    }
    return *roles;
}
