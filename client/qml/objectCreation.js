function createAsync(compStr, parentElem, callback) {
    let comp = Qt.createComponent(compStr + ".qml");
    let incubator = comp.incubateObject(parentElem, { visible: false }, Qt.Asynchronous);
    if (incubator.status !== Component.Ready) {
        incubator.onStatusChanged = callback;
    } else {
        callback(Component.Ready);
    }
}
