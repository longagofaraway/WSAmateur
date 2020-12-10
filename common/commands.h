#pragma once

#include <string>

class ExpectedCommand {
    std::string mCommand;
    size_t mCount = 0;
    size_t mMaxCount;

public:
    ExpectedCommand(const std::string &command, size_t maxCount = 0) : mCommand(command), mMaxCount(maxCount) {
        // 0 is unlimited
        if (mMaxCount == 0)
            mMaxCount = std::numeric_limits<size_t>::max();
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
