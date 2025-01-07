#pragma once

struct MoveParams {
    int targetPos{-1};
    bool reveal{false};
    bool enableGlobEncore{true};
    bool isRevealed{false};
    std::string purpose;
};
