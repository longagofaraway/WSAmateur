#include "deckMenuModel.h"

#include "deckUtils.h"

namespace {
int indexFromCoord(int row, int column, int columnCount) {
    return row * columnCount + column;
}
}

DeckMenuModel::DeckMenuModel() {
    decks.push_back(DeckMenuItem());
}

void DeckMenuModel::setDecks(std::vector<DeckMenuItem> &&newData) {
    newData.push_back(DeckMenuItem());
    beginResetModel();
    decks.swap(newData);
    endResetModel();
}

int DeckMenuModel::rowFromIndex(size_t index) const {
    return (static_cast<int>(index) / kColumnCount);
}

int DeckMenuModel::columnFromIndex(size_t index) const {
    return index % kColumnCount;
}

void DeckMenuModel::addDeck(const DeckList &deck, bool hasAllImages, bool startImageDownload) {
    DeckMenuItem item(deck, hasAllImages);
    if (startImageDownload && !hasAllImages)
        item.downloadInProgress = true;
    int currentRowCount = rowFromIndex(decks.size());
    beginInsertRows(QModelIndex(), currentRowCount, currentRowCount);

    if (decks.empty())
        decks.emplace_back(std::move(item));
    else
        decks.emplace(std::prev(decks.end()), std::move(item));

    endInsertRows();
}

const DeckList* DeckMenuModel::getDeck(int row, int column) {
    int vectorIndex = indexFromCoord(row, column, kColumnCount);
    if (static_cast<size_t>(vectorIndex) >= decks.size())
        return nullptr;

    return &decks.at(vectorIndex).deck;
}

const std::vector<DeckMenuItem>& DeckMenuModel::items() const {
    return decks;
}

void DeckMenuModel::setDowloadStarted(int row, int column) {
    int vectorIndex = indexFromCoord(row, column, kColumnCount);
    if (static_cast<size_t>(vectorIndex) >= decks.size())
        return;
    decks.at(vectorIndex).downloadInProgress = true;
    emit dataChanged(index(row, column), index(row, column), { DownloadRole });
}

void DeckMenuModel::dowloadFinished(QString deckName) {
    for (size_t i = 0; i < decks.size(); ++i) {
        if (decks[i].name == deckName) {
            decks[i].downloadInProgress = false;
            decks[i].hasAllImages = true;
            decks[i].thumbnail = getThumbnail(decks[i].deck);
            emit dataChanged(index(rowFromIndex(i), columnFromIndex(i)),
                             index(rowFromIndex(i), columnFromIndex(i)),
                             { ThumbnailRole, ImagesRole, DownloadRole });
        }
    }
}

void DeckMenuModel::updatePercent(QString deckName, int value) {
    for (size_t i = 0; i < decks.size(); ++i) {
        if (decks[i].name == deckName) {
            decks[i].percent = value;
            emit dataChanged(index(rowFromIndex(i), columnFromIndex(i)),
                             index(rowFromIndex(i), columnFromIndex(i)),
                             { PercentRole });
        }
    }
}

void DeckMenuModel::setErrorMessage(QString deckName, QString message) {
    for (size_t i = 0; i < decks.size(); ++i) {
        if (decks[i].name == deckName) {
            decks[i].errorMessage = message;
            decks[i].downloadInProgress = false;
            emit dataChanged(index(rowFromIndex(i), columnFromIndex(i)),
                             index(rowFromIndex(i), columnFromIndex(i)),
                             { ErrorMessageRole, DownloadRole });
        }
    }
}

int DeckMenuModel::rowCount(const QModelIndex &) const {
    return rowFromIndex(decks.size()) + 1;
}

int DeckMenuModel::columnCount(const QModelIndex &) const {
    return kColumnCount;
}

QVariant DeckMenuModel::data(const QModelIndex &index, int role) const {
    int vectorIndex = indexFromCoord(index.row(), index.column(), kColumnCount);
    bool indexValid = true;
    if (static_cast<size_t>(vectorIndex) >= decks.size())
        indexValid = false;

    switch (role) {
    case NameRole:
        return indexValid ? decks[vectorIndex].name : "";
    case ThumbnailRole:
        return indexValid ? decks[vectorIndex].thumbnail : "cardback";
    case LastElementRole:
        return vectorIndex == decks.size() - 1;
    case InvalidElementRole:
        return !indexValid;
    case ImagesRole:
        return indexValid ? decks[vectorIndex].hasAllImages : false;
    case PercentRole:
        return indexValid ? decks[vectorIndex].percent : 0;
    case DownloadRole:
        return indexValid ? decks[vectorIndex].downloadInProgress : false;
    case ErrorMessageRole:
        return indexValid ? decks[vectorIndex].errorMessage : "";
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
        (*roles)[ImagesRole] = "hasAllImages";
        (*roles)[PercentRole] = "percent";
        (*roles)[DownloadRole] = "downloadInProgress";
        (*roles)[ErrorMessageRole] = "errorMessage";
    }
    return *roles;
}

DeckMenuItem::DeckMenuItem() {
    name = "last element";
}

DeckMenuItem::DeckMenuItem(const DeckList &deck, bool hasAllImages)
    : hasAllImages(hasAllImages), deck(deck) {
    name = deck.qname();
    if (!hasAllImages)
        thumbnail = "cardback";
    else
        thumbnail = getThumbnail(deck);
}
