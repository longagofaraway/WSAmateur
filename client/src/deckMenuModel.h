#pragma once

#include <QAbstractTableModel>
#include <QQuickItem>

#include "deckList.h"

struct DeckMenuItem {
    QString name;
    QString thumbnail = "cardback";
    bool hasAllImages = false;
    DeckList deck;
    QString errorMessage;

    int percent = 0;
    bool downloadInProgress = false;

    DeckMenuItem();
    DeckMenuItem(const DeckList &deck, bool hasAllImages);
};

class DeckMenuModel : public QAbstractTableModel
{
    Q_OBJECT
private:
    std::vector<DeckMenuItem> decks;
    static constexpr int kColumnCount = 4;

public:
    enum TableRoles {
        NameRole = Qt::UserRole + 1,
        ThumbnailRole,
        LastElementRole,
        InvalidElementRole,
        ImagesRole,
        PercentRole,
        DownloadRole,
        ErrorMessageRole
    };
    Q_ENUM(TableRoles)

    DeckMenuModel();

    void setDecks(std::vector<DeckMenuItem> &&newData);
    void addDeck(const DeckList &deck, bool hasAllImages, bool startImageDownload = false);
    const DeckList* getDeck(int row, int column);
    const std::vector<DeckMenuItem>& items() const;
    void setDowloadStarted(int row, int column);
    void dowloadFinished(QString deckName);
    void updatePercent(QString deckName, int value);
    void setErrorMessage(QString deckName, QString message);

    int rowCount(const QModelIndex & = QModelIndex()) const override;
    int columnCount(const QModelIndex & = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    int rowFromIndex(size_t index) const;
    int columnFromIndex(size_t index) const;
};

