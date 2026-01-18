#pragma once

#include <QQuickItem>

class ApplicationData {
public:
    static ApplicationData& get();
    void setWorkingArea(QQuickItem* workingArea);
    QQuickItem* workingArea() { return workingArea_; }

private:
    ApplicationData() = default;

    QQuickItem* workingArea_;
};
