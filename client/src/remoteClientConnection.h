#pragma once

#include <vector>

#include <QTcpSocket>

#include "clientConnection.h"


class RemoteClientConnection : public ClientConnection
{
    Q_OBJECT
private:
    QTcpSocket *socket;
    std::vector<uint8_t> inputBuffer;
    bool messageInProgress = false;
    uint32_t messageLength;

public:
    RemoteClientConnection();

    void sendMessage(std::shared_ptr<CommandContainer> cont) override;
    void connectToHost(const QString &hostname, uint16_t port) override;

private slots:
    void onConnected();
    void readData();
    void onError(QAbstractSocket::SocketError);
};
