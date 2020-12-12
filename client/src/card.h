#pragma once

#include <string>

#include <QString>

class Card {
    std::string mCode;
    bool mGlow = false;

public:
    Card(std::string code) : mCode(code) {}

    bool glow() const { return mGlow; }
    void setGlow(bool glow) { mGlow = glow; }
    QString qcode() const { return QString::fromStdString(mCode); }
};
