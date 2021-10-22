#include "remoteClientConnection.h"

#include <QTimer>

#include "commandContainer.pb.h"
#include "serverMessage.pb.h"
#include "sessionCommand.pb.h"

#include "networkUtils.h"

namespace {
std::shared_ptr<CommandContainer> prepareSessionCommand(const ::google::protobuf::Message &cmd) {
    SessionCommand sessionCmd;
    sessionCmd.mutable_command()->PackFrom(cmd);

    auto cont = std::make_shared<CommandContainer>();
    cont->mutable_command()->PackFrom(sessionCmd);
    return cont;
}
}

RemoteClientConnection::RemoteClientConnection() {
    timer = new QTimer(this);
    timer->setInterval(keepalive * 1000);
    connect(timer, SIGNAL(timeout()), this, SLOT(ping()));

    socket = new QTcpSocket(this);
    socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
    connect(socket, &QTcpSocket::connected, this, &RemoteClientConnection::onConnected);
    connect(socket, &QTcpSocket::readyRead, this, &RemoteClientConnection::readData);
    connect(this, &RemoteClientConnection::sigDisconnectFromServer, this, &RemoteClientConnection::disconnectFromServer);
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
            this, &RemoteClientConnection::onError);
    // replace prev line with commented after github workflow has ubuntu 21 and qt 5.15
    //connect(socket, &QTcpSocket::errorOccurred, this, &RemoteClientConnection::onError);
}

void RemoteClientConnection::onConnected() {
    timeRunning = lastDataReceived = 0;
    timer->start();
}

void RemoteClientConnection::sendMessage(std::shared_ptr<CommandContainer> cont) {
#if GOOGLE_PROTOBUF_VERSION > 3001000
    uint32_t size = static_cast<uint32_t>(cont->ByteSizeLong());
#else
    uint32_t size = static_cast<uint32_t>(cont->ByteSize());
#endif

    QByteArray buf;
    buf.resize(size + 4);
    cont->SerializeToArray(buf.data() + 4, size);
    toBigEndian(size, buf.data());

    socket->write(buf);
}

void RemoteClientConnection::connectToHost(const QString &hostname, uint16_t port) {
    socket->connectToHost(hostname, port);
}

void RemoteClientConnection::readData() {
    lastDataReceived = timeRunning;
    QByteArray data = socket->readAll();

    inputBuffer.insert(inputBuffer.end(), data.data(), data.data() + data.size());

    do {
        if (!messageInProgress) {
            if (inputBuffer.size() >= 4) {
                messageLength = fromBigEndian(inputBuffer.data());
                inputBuffer.erase(inputBuffer.begin(), inputBuffer.begin() + 4);
                messageInProgress = true;
            } else {
                return;
            }
        }
        if (inputBuffer.size() < messageLength)
            return;

        auto newServerMessage = std::make_shared<ServerMessage>();
        bool status = newServerMessage->ParseFromArray(inputBuffer.data(), messageLength);

        inputBuffer.erase(inputBuffer.begin(), inputBuffer.begin() + messageLength);
        messageInProgress = false;

        if (!status) {
            qDebug() << "Parsing protobuf message failed";
            return;
        }

        emit messageReady(newServerMessage);
    } while (!inputBuffer.empty());
}

void RemoteClientConnection::ping() {
    int maxTime = timeRunning - lastDataReceived;

    if (maxTime >= (keepalive * maxTimeout)) {
        emit sigDisconnectFromServer();
    } else {
        sendMessage(prepareSessionCommand(CommandPing()));
        ++timeRunning;
    }
}

void RemoteClientConnection::onError(QAbstractSocket::SocketError) {
    qDebug() << socket->errorString();
    disconnectFromServer();
    emit connectionClosed();
}

void RemoteClientConnection::disconnectFromServer() {
    timer->stop();
    socket->close();

    messageInProgress = false;
    messageLength = 0;
    inputBuffer.clear();
}
