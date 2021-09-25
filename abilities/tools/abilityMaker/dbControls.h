#pragma once

#include <QQuickItem>

#include "dbManager.h"

class AbilityMaker;

class DbControls : public QObject
{
    Q_OBJECT
private:
    QQuickItem *qmlObject;
    AbilityMaker *maker;

    std::unique_ptr<DbManager> dbManager;

public:
    DbControls(AbilityMaker *maker_, QQuickItem *parent);
    ~DbControls();

signals:
    void sendMessage(QString msg);

private slots:
    void addAbility(QString code);
    void popAbility(QString code);
};

