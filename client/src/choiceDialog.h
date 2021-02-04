#pragma once

#include <vector>

#include <QAbstractListModel>
#include <QString>

class Game;

class QQuickItem;

class ChoiceDialogModel : public QAbstractListModel
{
    Q_OBJECT
private:
    std::vector<QString> mData;

public:
    enum Roles {
        TextRole = Qt::UserRole + 1
    };

    ChoiceDialogModel() : QAbstractListModel(nullptr) {}

    void clear();
    void setChoice(const std::vector<QString> &data);
    int rowCount(const QModelIndex & = QModelIndex()) const override { return static_cast<int>(mData.size()); };
    int count() const { return rowCount(); }
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

protected:
    QHash<int, QByteArray> roleNames() const override;
};

class ChoiceDialog {
    ChoiceDialogModel mModel;
    QQuickItem *mQmlObject;

public:
    ChoiceDialog(Game *game);

    void setData(const QString &header, const std::vector<QString> &data);
};
