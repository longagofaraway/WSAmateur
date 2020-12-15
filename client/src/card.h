#pragma once

#include <string>

#include <QString>

class Card {
    std::string mCode;
    bool mGlow = false;
    bool mSelected = false;

public:
    Card(std::string code) : mCode(code) {}

    bool glow() const { return mGlow; }
    bool selected() const { return mSelected; }
    void setGlow(bool glow) { mGlow = glow; }
    void setSelected(bool selected) { mSelected = selected; }
    QString qcode() const { return QString::fromStdString(mCode); }
};
