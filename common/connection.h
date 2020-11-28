#pragma once

#include <QObject>

class Connection : public QObject
{
    Q_OBJECT
public:
    explicit Connection(QObject *parent = nullptr)
        : QObject(parent) {}

    //virtual void connect() = 0;
    //virtual void close() = 0;
    virtual void sendMessage() = 0;

signals:
    void error();
    void messageReady();
};
