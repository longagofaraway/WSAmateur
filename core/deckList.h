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
    std::string mDeck;
public:
    DeckList(const std::string &deck);

    const std::vector<DeckCard>& cards() const { return mCards; }
    const std::string& deck() const { return mDeck; }

private:
    void readCards(QXmlStreamReader &xml);
    void readElement(QXmlStreamReader &xml);
};
