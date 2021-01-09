function createCard(cardParent, _component) {
    var comp = Qt.createComponent(_component);
    var card = comp.createObject(cardParent, {x: 0, y: 0});
    if (card === null)
        console.log("Error creating object");
    return card;
}
