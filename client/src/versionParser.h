#pragma once

#include <string>
#include <vector>

#include <QString>

std::vector<int> parseVersion(const std::string &version);
std::vector<int> parseVersion(const QString version);
