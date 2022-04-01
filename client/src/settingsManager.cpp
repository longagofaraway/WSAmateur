#include "settingsManager.h"

#include "filesystemPaths.h"

namespace {
const QString kProfile = "profile";
const QString kUsername = "username";

const QString kDeveloper = "developer";
const QString kLocalGameEnabled = "localgame";
}

SettingsManager::SettingsManager(QString settingsPath)
    : settings(settingsPath, QSettings::IniFormat) {
}

SettingsManager& SettingsManager::get() {
    static SettingsManager instance(paths::settingsPath());
    return instance;
}

QString SettingsManager::getUsername() {
    return getValue(kUsername, kProfile).toString();
}

void SettingsManager::setUsername(QString username) {
    setValue(username, kUsername, kProfile);
}

bool SettingsManager::hasUsername() {
    return hasValue(kUsername, kProfile);
}

bool SettingsManager::localGameEnabled() {
    return hasValue(kLocalGameEnabled, kDeveloper);
}

void SettingsManager::setValue(QVariant value, QString name, QString group) {
    if (!group.isEmpty()) {
        settings.beginGroup(group);
    }

    settings.setValue(name, value);

    if (!group.isEmpty()) {
        settings.endGroup();
    }
}

QVariant SettingsManager::getValue(QString name, QString group) {
    if (!group.isEmpty()) {
        settings.beginGroup(group);
    }

    QVariant value = settings.value(name);

    if (!group.isEmpty()) {
        settings.endGroup();
    }

    return value;
}

bool SettingsManager::hasValue(QString name, QString group) {
    if (!group.isEmpty()) {
        settings.beginGroup(group);
    }

    bool value = settings.contains(name);

    if (!group.isEmpty()) {
        settings.endGroup();
    }

    return value;
}
