#pragma once

#include <QSettings>

class SettingsManager
{
    QSettings settings;

    SettingsManager(QString settingsPath);
    SettingsManager(const SettingsManager&) = delete;
    SettingsManager& operator=(const SettingsManager&) = delete;

public:
    static SettingsManager& get();

    QString getUsername();
    void setUsername(QString username);
    bool hasUsername();
    bool localGameEnabled() ;

private:
    void setValue(QVariant value, QString name, QString group);
    QVariant getValue(QString name, QString group);
    bool hasValue(QString name, QString group);
};

