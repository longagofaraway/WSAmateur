#include "remoteClientConnection.h"

#include "commandContainer.pb.h"
#include "serverMessage.pb.h"

#include "networkUtils.h"

RemoteClientConnection::RemoteClientConnection() {
    socket = new QTcpSocket(this);
    socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
    connect(socket, &QTcpSocket::connected, this, &RemoteClientConnection::onConnected);
    connect(socket, &QTcpSocket::readyRead, this, &RemoteClientConnection::readData);
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
            this, &RemoteClientConnection::onError);
    // replace prev line with commented after github workflow has ubuntu 21 and qt 5.15
    //connect(socket, &QTcpSocket::errorOccurred, this, &RemoteClientConnection::onError);
}

void RemoteClientConnection::onConnected() {

}

void RemoteClientConnection::sendMessage(std::shared_ptr<CommandContainer> cont) {
    uint32_t size = static_cast<uint32_t>(cont->ByteSizeLong());

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

void RemoteClientConnection::onError(QAbstractSocket::SocketError) {
    qDebug() << socket->errorString();
}
