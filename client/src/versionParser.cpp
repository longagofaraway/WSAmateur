#include "versionParser.h"

#include <sstream>

std::vector<int> parseVersion(const std::string &version) {
    std::stringstream versionStream(version);
    std::vector<int> res;

    std::string tmp;
    while(getline(versionStream, tmp, '.')){
        res.push_back(std::stoi(tmp));
    }

    if (res.size() != 3)
        throw std::runtime_error("wrong version format");

    return res;
}

std::vector<int> parseVersion(const QString version) {
    return parseVersion(version.toStdString());
}
