#include "statusLine.h"

#include "abilityMaker.h"

AbilityMaker *maker;

void initStatusLine(AbilityMaker *m) {
    maker = m;
}

void statusLinePush(QString dir) {
    if (maker)
        maker->statusLinePush(dir);
}

void statusLinePop() {
    if (maker)
        maker->statusLinePop();
}

void deinitStatusLine() {
    maker = nullptr;
}
