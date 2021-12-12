#pragma once

#include <string>
#include <vector>

#include <QString>

class QString;
class QByteArray;
class QXmlStreamReader;

struct DeckCard {
    int count;
    std::string code;
    DeckCard(int num, std::string &&c)
        : count(num), code(std::move(c)) {}
};

class DeckList
{
    std::string name_;
    std::string comments_;
    std::vector<DeckCard> cards_;
    std::string deck_;
public:
    DeckList() {}

    bool fromXml(const std::string &deck_string);
    bool fromXml(const QString &deck_string);
    bool fromEncoreDecksResponse(const QByteArray &data);

    QString toXml() const;

    const std::vector<DeckCard>& cards() const { return cards_; }
    const std::string& deck() const { return deck_; }
    const std::string& name() const { return name_; }
    const QString qname() const { return QString::fromStdString(name_); }

private:
    void readCards(QXmlStreamReader &xml);
    bool parseXml(QXmlStreamReader &xml);
};
