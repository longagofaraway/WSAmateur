#include "choiceDialog.h"

#include <QQmlContext>

#include "game.h"

void ChoiceDialogModel::clear() {
    if (!mData.size())
        return;
    beginRemoveRows(QModelIndex(), 0, static_cast<int>(mData.size()) - 1);
    mData.clear();
    endRemoveRows();
}

void ChoiceDialogModel::setChoice(const std::vector<QString> &data) {
    clear();
    beginInsertRows(QModelIndex(), 0, static_cast<int>(data.size()) - 1);
    mData = data;
    endInsertRows();
}

QVariant ChoiceDialogModel::data(const QModelIndex &index, int role) const {
    if (static_cast<size_t>(index.row()) >= mData.size())
        return QVariant();

    switch(role) {
    case TextRole:
        return mData[index.row()];
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> ChoiceDialogModel::roleNames() const {
    static QHash<int, QByteArray> *roles;
    if (!roles) {
        roles = new QHash<int, QByteArray>;
        (*roles)[TextRole] = "text";
    }
    return *roles;
}

ChoiceDialog::ChoiceDialog(Game *game) {
    QQmlComponent component(game->engine(), "qrc:/qml/ChoiceDialog.qml");
    QQmlContext *context = new QQmlContext(game->context(), game);
    context->setContextProperty("innerModel", QVariant::fromValue(&mModel));
    QObject *obj = component.create(context);
    mQmlObject = qobject_cast<QQuickItem*>(obj);
    mQmlObject->setParentItem(game);
    mQmlObject->setParent(game);
}

void ChoiceDialog::setData(const QString &header, const std::vector<QString> &data) {
    mQmlObject->setProperty("mHeaderText", header);
    mQmlObject->setProperty("state", "active");

    if (data.size() && data[0].size() > 16)
        mQmlObject->setProperty("mLongtext", true);
    else
        mQmlObject->setProperty("mLongtext", false);
    mModel.setChoice(data);
}
