#pragma once

#include <string>
#include <vector>

class QXmlStreamReader;

struct DeckCard {
    size_t number;
    std::string code;
    DeckCard(size_t num, std::string &&c)
        : number(num), code(std::move(c)) {}
};

class DeckList
{
    std::string mName;
    std::string mComments;
    std::vector<DeckCard> mCards;
public:
    DeckList(const std::string &deck);

    std::vector<DeckCard>& cards() { return mCards; }

private:
    void readCards(QXmlStreamReader &xml);
    void readElement(QXmlStreamReader &xml);
};
