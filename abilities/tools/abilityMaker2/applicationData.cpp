#include "applicationData.h"


ApplicationData& ApplicationData::get() {
    static ApplicationData data;
    return data;
}

void ApplicationData::setWorkingArea(QQuickItem* workingArea) {
    workingArea_ = workingArea;
}
