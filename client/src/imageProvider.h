#pragma once

#include <QQuickAsyncImageProvider>
#include <QThreadPool>

class AsyncImageResponse : public QQuickImageResponse, public QRunnable
{
    QString id;
    QSize requestedSize;
    QImage image;

    public:
        AsyncImageResponse(const QString &id, const QSize &requestedSize);

        QQuickTextureFactory *textureFactory() const override {
            return QQuickTextureFactory::textureFactoryForImage(image);
        }

        void run() override;

    private:
        bool loadFromDisk();
};

class AsyncImageProvider : public QQuickAsyncImageProvider
{
    QThreadPool pool;

public:
    QQuickImageResponse *requestImageResponse(const QString &id, const QSize &requestedSize) override
    {
        AsyncImageResponse *response = new AsyncImageResponse(id, requestedSize);
        pool.start(response);
        return response;
    }

};
