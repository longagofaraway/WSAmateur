#pragma once

#include <vector>

#include <QTcpSocket>

#include "clientConnection.h"

class QTimer;

class RemoteClientConnection : public ClientConnection
{
    Q_OBJECT
private:
    QTcpSocket *socket;
    std::vector<uint8_t> inputBuffer;
    bool messageInProgress = false;
    uint32_t messageLength;

    QTimer *timer;
    int timeRunning = 0;
    int lastDataReceived = 0;
    const int keepalive = 5;
    const int maxTimeout = 10;

public:
    RemoteClientConnection();

    void sendMessage(std::shared_ptr<CommandContainer> cont) override;
    void connectToHost(const QString &hostname, uint16_t port) override;

signals:
    void sigDisconnectFromServer();

private slots:
    void onConnected();
    void readData();
    void ping();
    void onError(QAbstractSocket::SocketError);
    void disconnectFromServer();
};
