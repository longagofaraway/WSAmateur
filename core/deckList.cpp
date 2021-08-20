#include "deckList.h"

#include <stdexcept>
#include <QXmlStreamReader>

#include <QDebug>

void DeckList::readCards(QXmlStreamReader &xml) {
    while (!xml.atEnd()) {
        xml.readNext();
        if(xml.isStartElement()) {
            if (xml.name() == QString("card")) {
                int number = xml.attributes().value("number").toInt();
                if (!number)
                    continue;
                mCards.emplace_back(DeckCard(number, xml.attributes().value("code").toString().toStdString()));
            }
        }
    }
}

DeckList::DeckList(const std::string &deck)
    : mDeck(deck)
{
    QXmlStreamReader xml;

    xml.addData(deck.c_str());

    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            if (xml.name() != QString("deck"))
                return;
            while (!xml.atEnd()) {
                xml.readNext();
                if (xml.isStartElement()) {
                    if (xml.name() == QString("deckname"))
                        mName = xml.readElementText().toStdString();
                    else if (xml.name() == QString("comments"))
                        mComments = xml.readElementText().toStdString();
                    else if (xml.name() == QString("main"))
                        readCards(xml);
                }
            }
        }
    }

    if (xml.hasError()) {

    }

}
