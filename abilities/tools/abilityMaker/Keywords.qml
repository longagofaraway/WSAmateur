import QtQuick 2.0
import QtQuick.Controls 2.12

ComboBox {
    signal selected(int index, int keywordIndex)
    property int index

    model: ["Encore", "Cxcombo", "Brainstorm"]
    onCurrentIndexChanged: selected(index, currentIndex);
}
