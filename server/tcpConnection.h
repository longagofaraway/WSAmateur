#pragma once

#include <QTcpSocket>

#include "connection.h"

class TcpConnection : public Connection
{
    Q_OBJECT
private:
    QTcpSocket *socket;
    qintptr socketDescriptor;
    std::vector<uint8_t> inputBuffer;
    bool messageInProgress = false;
    uint32_t messageLength = 0;

public:
    TcpConnection(qintptr socketDescriptor);
    ~TcpConnection();

    void init() override;
    void sendMessage(std::shared_ptr<ServerMessage> message) override;
    void flush() override;

private slots:
    void read();
    void catchSocketError(QAbstractSocket::SocketError socketError);
    void catchSocketDisconnected();
};
