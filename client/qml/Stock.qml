import QtQuick 2.12

import wsamateur.cardModel 1.0

ListView {
   id: stock

   property CardModel mModel: innerModel
   property bool opponent: false
   anchors.left: opponent ? undefined : root.left
   anchors.leftMargin: opponent ? 0 : (-root.width * 0.03)
   anchors.right: opponent ?  root.right : undefined
   anchors.rightMargin: opponent ? (-root.width * 0.03) : 0
   anchors.top: opponent ? undefined : root.verticalCenter
   anchors.bottom: opponent ? root.verticalCenter : undefined

   function getStockHeight()  {
       if (!stock.count)
           return 0;
       return (stock.count - 1) * root.cardHeight * 0.2 + root.cardHeight;
   }

   width: root.cardHeight
   height: getStockHeight()

   rotation: opponent ? 180 : 0
   spacing: -root.cardHeight * 0.8

   model: mModel
       /*ListModel {
       ListElement { code: "q1" }
       ListElement { code: "q2" }
       ListElement { code: "q2" }
   }*/
   delegate: Component {
       Card {
           rotation: -90
           source: "image://imgprov/cardback";
       }
   }

   /*function getXForNewCard() { return clockView.x + clockView.count * root.cardWidth * 2/3; }
   function getYForNewCard() { return clockView.y; }
   function getXForCard() { return waitingRoom.x; }
   function getYForCard() { return waitingRoom.y; }*/
}
