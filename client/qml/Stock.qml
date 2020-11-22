import QtQuick 2.12

ListView {
   id: stock

   property bool opponent: false
   anchors.left: opponent ? undefined : parent.left
   anchors.leftMargin: opponent ? 0 : (-root.width * 0.03)
   anchors.right: opponent ?  parent.right : undefined
   anchors.rightMargin: opponent ? (-root.width * 0.03) : 0
   anchors.top: opponent ? undefined : parent.verticalCenter
   anchors.bottom: opponent ? parent.verticalCenter : undefined

   function getStockHeight()  {
       if (!stock.count)
           return 0;
       return (stock.count - 1) * root.cardHeight * 0.2 + root.cardHeight;
   }

   width: root.cardHeight
   height: getStockHeight()

   rotation: opponent ? 180 : 0
   spacing: -root.cardHeight * 0.8

   model: ListModel {
       ListElement { code: "q1" }
       ListElement { code: "q2" }
       ListElement { code: "q2" }
   }
   delegate: Component {
       Card {
           rotation: -90
           source: "qrc:///resources/images/cardback"
       }
   }
}
