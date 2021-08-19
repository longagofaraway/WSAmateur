#include "tcpConnection.h"

#include "commandContainer.pb.h"
#include "serverMessage.pb.h"

namespace {
uint32_t fromBigEndian(const uint8_t *data) {
    uint32_t val = 0;
    for (int i = 0; i < sizeof(val); ++i)
        val += data[i] << (sizeof(val) - i - 1) * 8;
    return val;
}

void toBigEndian(uint32_t val, char *data) {
    for (int i = 0; i < sizeof(val); ++i) {
        data[sizeof(val) - i - 1] = (val >> i * 8) & 0xFF;
    }
}
}

TcpConnection::TcpConnection(qintptr socketDescriptor)
    : socketDescriptor(socketDescriptor) {
    socket = new QTcpSocket(this);
    socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
    connect(socket, &QTcpSocket::readyRead, this, &TcpConnection::read);
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
            this, &TcpConnection::catchSocketError);
    // the next line should replace the previous, when there'll be ubuntu 21 and qt5.15 on github workflows
    //connect(socket, &QTcpSocket::errorOccurred, this, &TcpConnection::catchSocketError);
    connect(socket, &QTcpSocket::disconnected, this, &TcpConnection::catchSocketDisconnected);
}

TcpConnection::~TcpConnection() {
    qDebug("connection deleted");
}

void TcpConnection::init() {
    if (!socket->setSocketDescriptor(socketDescriptor))
        qDebug() << socket->errorString();
}

void TcpConnection::sendMessage(std::shared_ptr<ServerMessage> message) {
    QByteArray buf;
    uint32_t size = message->ByteSize();
    buf.resize(size + sizeof(size));
    toBigEndian(size, buf.data());
    message->SerializePartialToArray(buf.data() + sizeof(size), size);
    socket->write(buf);
}

void TcpConnection::flush() {
    socket->flush();
}

void TcpConnection::read() {
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


        auto cc = std::make_shared<CommandContainer>();
        bool status = false;
        try {
            status = cc->ParseFromArray(inputBuffer.data(), messageLength);
        } catch (const std::exception &e) {
            qDebug() << "Exception in protobuf message parsing: " << e.what();
        }

        inputBuffer.erase(inputBuffer.begin(), inputBuffer.begin() + messageLength);
        messageInProgress = false;

        if (!status) {
            qDebug() << "Error in protobuf message parsing";
            return;
        }

        emit messageReady(cc);
    } while (!inputBuffer.empty());
}

void TcpConnection::catchSocketError(QAbstractSocket::SocketError socketError) {
    qDebug() << socket->errorString();
    emit connectionClosed();
}

void TcpConnection::catchSocketDisconnected() {
    emit connectionClosed();
}
