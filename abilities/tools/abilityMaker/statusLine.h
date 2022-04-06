#pragma once

#include <QString>

class AbilityMaker;

void initStatusLine(AbilityMaker *m);
void deinitStatusLine();
void statusLinePush(QString dir);
void statusLinePop();
