#pragma once


enum class State : uint8_t {
    Standing = 1,
    Rested,
    Reversed
};

enum class Owner : uint8_t {
    Player = 1,
    Opponent
};

enum class AsnPlayer : uint8_t {
    Player = 1,
    Opponent,
    Both
};

enum class AsnZone : uint8_t {
    Stage = 1,
    WaitingRoom,
    Deck,
    Clock,
    Hand,
    Memory,
    Stock,
    Level
};

enum class Position : uint8_t {
    NotSpecified = 0,
    Top,
    Bottom,
    FrontRow,
    BackRow,
    EmptySlotFrontRow,
    EmptySlotBackRow,
    EmptySlot,
    SlotThisWasIn
};

struct Place {
    Position pos;
    AsnZone zone;
    Owner owner;
};
