#include "componentIdParser.h"

#include <stdexcept>

#include <QStringList>


std::tuple<QString, QString> parseComponentId(QString componentId) {
    QStringList parsed = componentId.split("/");
    if (parsed.size() != 3)
        throw std::runtime_error("can't parse component id"+componentId.toStdString());
    return std::make_tuple(parsed[1], parsed[2]);
}
