#include <QCoreApplication>

#include "cardDatabase.h"
#include "networkConnectionManager.h"
#include "server.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    try {
        // init db
        CardDatabase::get();
    } catch (const std::exception &e) {
        qDebug() << e.what();
        throw;
    }

    int threadNum = 4;
    int tcpPort = 7474;
    auto connectionManager = std::make_unique<NetworkConnectionManager>(threadNum, tcpPort);

    auto server = new Server(std::move(connectionManager));

    QObject::connect(server, SIGNAL(destroyed()), &app, SLOT(quit()), Qt::QueuedConnection);

    return app.exec();
}
