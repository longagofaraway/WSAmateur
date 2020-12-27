#include "deckList.h"

#include <exception>
#include <QXmlStreamReader>

#include <QDebug>

void DeckList::readCards(QXmlStreamReader &xml) {
    while (!xml.atEnd()) {
        xml.readNext();
        if(xml.isStartElement()) {
            if (xml.name() == "card") {
                int number = xml.attributes().value("number").toInt();
                if (!number)
                    throw std::runtime_error("deck parsing error");
                mCards.emplace_back(DeckCard(number, xml.attributes().value("code").toString().toStdString()));
            }
        }
    }
}

DeckList::DeckList(const std::string &deck)
{
    QXmlStreamReader xml;

    xml.addData(deck.c_str());

    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            if (xml.name() != "deck")
                return;
            while (!xml.atEnd()) {
                xml.readNext();
                if (xml.isStartElement()) {
                    if (xml.name() == "deckname")
                        mName = xml.readElementText().toStdString();
                    else if (xml.name() == "comments")
                        mComments = xml.readElementText().toStdString();
                    else if (xml.name() == "main")
                        readCards(xml);
                }
            }
        }
    }

    if (xml.hasError()) {

    }

}
