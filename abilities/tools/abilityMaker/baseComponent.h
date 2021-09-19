#pragma once

#include <QQuickItem>

#include "abilities.h"

class BaseComponent : public QObject
{
    Q_OBJECT
protected:
    QQuickItem *qmlObject;

public:
    explicit BaseComponent(const QString &moduleName, QQuickItem *parent);
    virtual ~BaseComponent();

signals:
    void close();

protected slots:
    virtual void componentReady() = 0;

signals:

};

