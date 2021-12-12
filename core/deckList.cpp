#include "deckList.h"

#include <stdexcept>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QXmlStreamReader>
#include <QDebug>

namespace {

QString parseSerial(QJsonObject jsonCard)
{
    if (!(jsonCard.contains("side") && jsonCard.contains("set") && jsonCard.contains("sid") &&
          jsonCard.contains("release")))
        return "";

    QString sid = jsonCard["sid"].toString();
    if (sid.startsWith("E", Qt::CaseInsensitive))
        sid = sid.mid(1);
    if (sid.size() >= 2 && (sid[1] == "e" || sid[1] == "E"))
        sid = sid.remove(1, 1);
    return jsonCard["set"].toString() + "/" + jsonCard["side"].toString() + jsonCard["release"].toString() + "-" + sid;
}

}

void DeckList::readCards(QXmlStreamReader &xml) {
    while (!xml.atEnd()) {
        xml.readNext();
        if(xml.isStartElement()) {
            if (xml.name() == QString("card")) {
                int number = xml.attributes().value("number").toInt();
                if (!number)
                    continue;
                cards_.emplace_back(DeckCard(number, xml.attributes().value("code").toString().toStdString()));
            }
        }
    }
}

bool DeckList::parseXml(QXmlStreamReader &xml) {
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            if (xml.name() != QString("deck"))
                return false;
            while (!xml.atEnd()) {
                xml.readNext();
                if (xml.isStartElement()) {
                    if (xml.name() == QString("deckname"))
                        name_ = xml.readElementText().toStdString();
                    else if (xml.name() == QString("comments"))
                        comments_ = xml.readElementText().toStdString();
                    else if (xml.name() == QString("main"))
                        readCards(xml);
                }
            }
        }
    }

    return !xml.hasError();
}

bool DeckList::fromXml(const std::string &deck_string) {
    deck_ = deck_string;
    QXmlStreamReader xml;

    xml.addData(deck_.c_str());
    return parseXml(xml);
}

bool DeckList::fromXml(const QString &deck_string) {
    deck_ = deck_string.toStdString();
    QXmlStreamReader xml;

    xml.addData(deck_string);
    return parseXml(xml);
}

bool DeckList::fromEncoreDecksResponse(const QByteArray &data) {
    QJsonParseError jsonError;
    QJsonDocument jsonResponse = QJsonDocument::fromJson(data, &jsonError);
    if (jsonError.error != QJsonParseError::NoError)
        return false;

    QJsonObject json = jsonResponse.object();
    if (!json.contains("name") || !json["name"].isString())
        return false;

    QString deckName = json["name"].toString();

    if (!json.contains("description") || !json["description"].isString())
        return false;

    QString description = json["description"].toString();
    description = description.replace("\n", "\n// ");

    if (!json.contains("cards") || !json["cards"].isArray())
        return false;

    QHash<QString, int> map;
    QJsonArray jsonCards = json["cards"].toArray();
    for (int i = 0; i < jsonCards.size(); ++i) {
        map[parseSerial(jsonCards[i].toObject())]++;
    }

    name_ = deckName.toStdString();
    comments_ = description.toStdString();

    for (auto it = map.begin(); it != map.end(); ++it) {
        cards_.emplace_back(it.value(), it.key().toStdString());
    }

    return true;
}

QString DeckList::toXml() const {
    QString xmlDeck;
    QXmlStreamWriter xml(&xmlDeck);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();

    xml.writeStartElement("deck");
    xml.writeAttribute("version", "1");
    xml.writeTextElement("deckname", QString::fromStdString(name_));
    xml.writeTextElement("comments", QString::fromStdString(comments_));

    xml.writeStartElement("main");
    for (const auto &card: cards_){
        xml.writeEmptyElement("card");
        xml.writeAttribute("number", QString::number(card.count));
        xml.writeAttribute("name", QString::fromStdString(card.code));
    }
    xml.writeEndElement();

    xml.writeEndElement();
    xml.writeEndDocument();
    return xmlDeck;
}
