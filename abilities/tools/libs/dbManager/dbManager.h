#pragma once

#include <QSqlDatabase>
#include <QString>

#include "abilities.h"

class DbManager
{
    QSqlDatabase db;

public:
    DbManager(QString path);
    ~DbManager();

    void addAbility(QString code, const asn::Ability ability);
    void popAbility(QString code);
};
