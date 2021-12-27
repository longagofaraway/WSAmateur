#include "imageProvider.h"

#include <QImageReader>
#include <QStandardPaths>

#include "filesystemPaths.h"


AsyncImageResponse::AsyncImageResponse(const QString &id, const QSize &requestedSize)
 : id(id), requestedSize(requestedSize) {
    setAutoDelete(false);
}

void AsyncImageResponse::run() {
    if (id == "cardback") {
        image = QImage(":/resources/images/cardback");
        emit finished();
        return;
    }

    if (loadFromDisk()) {
        emit finished();
        return;
    }

    image = QImage(50, 50, QImage::Format_RGB32);
    image.fill(Qt::blue);
    if (requestedSize.isValid())
        image = image.scaled(requestedSize);

    emit finished();
}

bool AsyncImageResponse::loadFromDisk() {
    auto fileName = paths::cardImagePath(id);

    QImageReader imgReader;
    imgReader.setDecideFormatFromContent(true);
    imgReader.setFileName(fileName);
    if (!imgReader.read(&image))
        return false;
    if (requestedSize.isValid())
        image = image.scaled(requestedSize);

    return true;
}
