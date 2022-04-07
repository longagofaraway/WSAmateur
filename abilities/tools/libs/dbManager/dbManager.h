#pragma once

#include <QSqlDatabase>
#include <QString>

#include "abilities.h"

class DbManager
{
    QSqlDatabase db;

public:
    DbManager();
    ~DbManager();

    void addAbility(QString code, const asn::Ability ability);
    void popAbility(QString code);
    asn::Ability getAbility(QString code, int pos);
    void editAbility(QString code, int pos, const asn::Ability ability);

private:
    void prepareDbFile();
};

