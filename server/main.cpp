#include <QCoreApplication>

#include <version_string.h>

#include "cardDatabase.h"
#include "imageLinks.h"
#include "networkConnectionManager.h"
#include "server.h"
#include "serverLogger.h"

ServerLogger *logger;
QThread *loggerThread;

void initLogger() {
    loggerThread = new QThread();
    loggerThread->setObjectName("logger");
    logger = new ServerLogger();
    logger->moveToThread(loggerThread);
    loggerThread->start();
    QMetaObject::invokeMethod(logger, "startLog", Qt::BlockingQueuedConnection);
}

void deinitLogger() {
    logger->deleteLater();
    loggerThread->wait();
    delete loggerThread;
}

void myMessageOutput(QtMsgType /*type*/, const QMessageLogContext &, const QString &msg) {
    logger->logMessage(msg);
}

int main(int argc, char *argv[]) {
    if (argc == 2 && std::string(argv[1]) == "-v") {
        std::cout << "running version" << VERSION_STRING;
        return 0;
    }

    QCoreApplication app(argc, argv);

    try {
        // init db
        CardDatabase::get().init();
    } catch (const std::exception &e) {
        qWarning() << e.what();
        throw;
    }

    if (!ImageLinks::get().loadFile()){
        qWarning() << "couldn't load image links file";
    }

    initLogger();

    qInstallMessageHandler(myMessageOutput);

    int threadNum = 4;
    int tcpPort = 7474;
    auto connectionManager = std::make_unique<NetworkConnectionManager>(threadNum, tcpPort);

    auto server = new Server(std::move(connectionManager));

    QObject::connect(server, SIGNAL(destroyed()), &app, SLOT(quit()), Qt::QueuedConnection);

    int retval = app.exec();

    deinitLogger();

    return retval;
}
