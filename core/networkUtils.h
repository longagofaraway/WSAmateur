#pragma once

inline uint32_t fromBigEndian(const uint8_t *data) {
    uint32_t val = 0;
    for (int i = 0; i < sizeof(val); ++i)
        val += data[i] << (sizeof(val) - i - 1) * 8;
    return val;
}

inline void toBigEndian(uint32_t val, char *data) {
    for (int i = 0; i < sizeof(val); ++i) {
        data[sizeof(val) - i - 1] = (val >> i * 8) & 0xFF;
    }
}
