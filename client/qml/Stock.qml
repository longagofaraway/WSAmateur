import QtQuick 2.12

import wsamateur.cardModel 1.0

ListView {
   id: stock

   property CardModel mModel: innerModel
   property bool opponent: false
   property bool hidden: true
   property real mMargin: root.cardHeight * 0.2

   x: opponent ? (root.width * 0.962) : (-root.width * 0.03)
   y: opponent ? (root.height / 2 - stock.contentHeight) : (root.height / 2)
   interactive: false
   width: root.cardWidth
   height: getStockHeight()

   rotation: opponent ? 180 : 0
   spacing: -root.cardHeight * 0.8

   model: mModel
   delegate: Component {
       Card {
           rotation: -90
           mSource: "cardback";
       }
   }

   function getStockHeight()  {
       if (!stock.count)
           return 0;
       return (stock.count - 1) * mMargin + root.cardHeight;
   }

   function getXForNewCard() { return stock.x; }
   function getYForNewCard() {
       if (opponent)
           return root.height / 2 - stock.count * mMargin - root.cardHeight;
       return stock.y + stock.count * mMargin;
   }
   function getXForCard() { return stock.x; }
   function getYForCard() {
       if (opponent)
           return root.height / 2 - (stock.count ? (stock.count - 1) : 0) * mMargin - root.cardHeight;
       return stock.y + (stock.count ? (stock.count - 1) : 0) * mMargin;
   }
   function addCard(code, targetId) { stock.mModel.addCard(); }
}
