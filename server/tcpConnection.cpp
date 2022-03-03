#include "tcpConnection.h"

#include "commandContainer.pb.h"
#include "serverMessage.pb.h"

#include "networkUtils.h"

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
}

void TcpConnection::init() {
    if (!socket->setSocketDescriptor(socketDescriptor))
        qDebug() << socket->errorString();
}

void TcpConnection::sendMessage(std::shared_ptr<ServerMessage> message) {
    QByteArray buf;
#if GOOGLE_PROTOBUF_VERSION > 3001000
    uint32_t size = static_cast<uint32_t>(message->ByteSizeLong());
#else
    uint32_t size = static_cast<uint32_t>(message->ByteSize());
#endif
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
