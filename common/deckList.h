#pragma once

#include <string>
#include <vector>

class QXmlStreamReader;

struct DeckCard {
    int count;
    std::string code;
    DeckCard(int num, std::string &&c)
        : count(num), code(std::move(c)) {}
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
