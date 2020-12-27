#pragma once

#include <string>

class ExpectedCommand {
    std::string mCommand;
    int mCount = 0;
    int mMaxCount;

public:
    ExpectedCommand(const std::string &command, int maxCount = 0) : mCommand(command), mMaxCount(maxCount) {
        // 0 is unlimited
        if (mMaxCount == 0)
            mMaxCount = std::numeric_limits<int>::max();
    }
    const std::string& command() const { return mCommand; }
    bool commandArrived() {
        if (++mCount == mMaxCount)
            return true;
        return false;
    }
};

inline bool operator==(const ExpectedCommand &lhs, const ExpectedCommand &rhs) {
    if (lhs.command() == rhs.command())
        return true;
    return false;
}
