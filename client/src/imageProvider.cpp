#include "imageProvider.h"

#include <QImageReader>
#include <QStandardPaths>


AsyncImageResponse::AsyncImageResponse(const QString &id, const QSize &requestedSize)
 : mId(id), mRequestedSize(requestedSize) {
    setAutoDelete(false);
}

void AsyncImageResponse::run() {
    if (mId == "cardback") {
        mImage = QImage(":/resources/images/cardback");
        emit finished();
        return;
    }

    if (loadFromDisk()) {
        emit finished();
        return;
    }

    mImage = QImage(50, 50, QImage::Format_RGB32);
    mImage.fill(Qt::blue);
    if (mRequestedSize.isValid())
        mImage = mImage.scaled(mRequestedSize);

    emit finished();
}

bool AsyncImageResponse::loadFromDisk() {
    QString appData = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    appData += "/downloadedPics";

    auto start = mId.indexOf('/');
    auto end = mId.indexOf('-');
    QString set = mId.mid(start, end - start);
    appData += "/" + set;

    mId.remove('/');
    appData += "/" + mId;

    QImageReader imgReader;
    imgReader.setDecideFormatFromContent(true);
    imgReader.setFileName(appData);
    if (!imgReader.read(&mImage))
        return false;
    if (mRequestedSize.isValid())
        mImage = mImage.scaled(mRequestedSize);

    return true;
}
