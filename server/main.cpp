#include <QCoreApplication>

#include "networkConnectionManager.h"
#include "server.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    int threadNum = 4;
    int tcpPort = 7474;
    auto connectionManager = std::make_unique<NetworkConnectionManager>(threadNum, tcpPort);

    auto server = std::make_unique<Server>(std::move(connectionManager));

    return app.exec();
}
