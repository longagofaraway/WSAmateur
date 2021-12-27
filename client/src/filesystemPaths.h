#pragma once

#include <QString>

namespace paths {
QString imageLinksPath();
QString settingsPath();
QString cardImagePath(const std::string &code);
QString cardImagePath(QString code);
bool cardImageExists(const std::string &code);
}
