#pragma once

#include <cstdint>

namespace asn {

enum class State : uint8_t {
    Standing = 1,
    Rested,
    Reversed
};

enum class Owner : uint8_t {
    Player = 1,
    Opponent
};

enum class Player : uint8_t {
    NotSpecified = 0,
    Player,
    Opponent,
    Both
};

enum class Zone : uint8_t {
    NotSpecified = 0,
    Stage,
    WaitingRoom,
    Deck,
    Clock,
    Hand,
    Memory,
    Stock,
    Level,
    Climax
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
    SlotThisWasInRested,
    SlotThisWasIn,
    SlotTargetWasIn
};

struct Place {
    Position pos;
    Zone zone;
    Player owner;
};

enum class PlaceType : uint8_t {
    Selection = 1,
    SpecificPlace,
    LastMovedCards
};

enum class FaceOrientation : uint8_t {
    FaceUp = 1,
    FaceDown
};

}
