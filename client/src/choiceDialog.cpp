#include "choiceDialog.h"

#include <QQmlContext>

#include "game.h"
#include "player.h"

void ChoiceDialogModel::clear() {
    if (!mData.size())
        return;
    beginRemoveRows(QModelIndex(), 0, static_cast<int>(mData.size()) - 1);
    mData.clear();
    endRemoveRows();
}

void ChoiceDialogModel::addRow(const QString &data) {
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    mData.push_back(data);
    endInsertRows();
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

ChoiceDialogBase::ChoiceDialogBase(Game *game) : mPlayer(game->player()) {
    QQmlComponent component(game->engine(), "qrc:/qml/ChoiceDialog.qml");
    QQmlContext *context = new QQmlContext(game->context(), this);
    context->setContextProperty("innerModel", QVariant::fromValue(&mModel));
    QObject *obj = component.create(context);
    mQmlObject = qobject_cast<QQuickItem*>(obj);
    mQmlObject->setParentItem(game);
    mQmlObject->setParent(this);
    mQmlObject->connect(mQmlObject, SIGNAL(destroySignal()), this, SLOT(destroy()));
}

ChoiceDialogBase::~ChoiceDialogBase() noexcept {
    mQmlObject->disconnect(mQmlObject, SIGNAL(destroySignal()), this, SLOT(destroy()));
}

void ChoiceDialogBase::destroy() {
    mPlayer->resetChoiceDialog();
}

ChoiceDialog::ChoiceDialog(Game *game) : ChoiceDialogBase(game) {
    mQmlObject->connect(mQmlObject, SIGNAL(choiceMade(int)), this, SLOT(processChoice(int)));
}

ChoiceDialog::~ChoiceDialog() noexcept {
    mQmlObject->disconnect(mQmlObject, SIGNAL(choiceMade(int)), this, SLOT(processChoice(int)));
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

void ChoiceDialog::processChoice(int choice) {
    mPlayer->sendChoice(choice);
}

ActChoiceDialog::ActChoiceDialog(Game *game) : ChoiceDialogBase(game) {
    mQmlObject->connect(mQmlObject, SIGNAL(choiceMade(int)), this, SLOT(processChoice(int)));
}

void ActChoiceDialog::addAbility(int cardPos, int abilityId, const QString &text) {
    mAbilities.emplace_back(AbilityData{abilityId, cardPos});
    mModel.addRow(text);
}

void ActChoiceDialog::show() {
    mQmlObject->setProperty("mHeaderText", "Play Act ability");
    mQmlObject->setProperty("mLongtext", true);
    mQmlObject->setProperty("mCancelable", true);
    mQmlObject->setProperty("state", "active");
}

void ActChoiceDialog::processChoice(int choice) {
    mPlayer->sendPlayActAbility(mAbilities[choice].cardPos, mAbilities[choice].abilityId);
}
