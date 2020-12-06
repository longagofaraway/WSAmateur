#include "cardDatabase.h"

CardDatabase::CardDatabase() {
    CardInfo info;
    info.setCode("IMC/W43-009");
    info.setLevel(1);
    info.setCost(0);
    info.setSoul(1);
    info.setPower(4000);
    info.setName("prr");
    info.setType("CH");
    info.setColor('Y');
    info.setTraits({ "Music", "Happy Happy" });
    mDb.emplace(info.code(), info);

    CardInfo info2;
    info2.setCode("IMC/W43-046");
    info2.setLevel(0);
    info2.setCost(0);
    info2.setSoul(1);
    info2.setPower(1500);
    info2.setName("prru");
    info2.setType("CH");
    info2.setColor('R');
    info2.setTraits({ "Music" });
    mDb.emplace(info2.code(), info2);

    CardInfo info3;
    info3.setCode("IMC/W43-111");
    info3.setLevel(1);
    info3.setCost(0);
    info3.setSoul(1);
    info3.setPower(5000);
    info3.setName("prru");
    info3.setType("CH");
    info3.setColor('B');
    info3.setTraits({ "Music" });
    mDb.emplace(info3.code(), info3);

    CardInfo info4;
    info4.setCode("IMC/W43-091");
    info4.setLevel(3);
    info4.setCost(2);
    info4.setSoul(2);
    info4.setPower(10000);
    info4.setName("prru");
    info4.setType("CH");
    info4.setColor('B');
    info4.setTraits({ "Music" });
    info4.setTriggers({ Soul });
    mDb.emplace(info4.code(), info4);

    CardInfo info5;
    info5.setCode("IMC/W43-127");
    info5.setLevel(0);
    info5.setCost(0);
    info5.setSoul(0);
    info5.setPower(0);
    info5.setName("prru");
    info5.setType("CX");
    info5.setColor('B');
    info5.setTraits({ "Music" });
    info4.setTriggers({ Door });
    mDb.emplace(info5.code(), info5);
}

CardDatabase& CardDatabase::get() {
    static CardDatabase instance;
    return instance;
}

CardInfo CardDatabase::getCard(const std::string &code) {
    if (!mDb.count(code))
        throw std::runtime_error("card not found");

    return mDb.at(code);
}
