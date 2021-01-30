#pragma once

#include <vector>

inline uint32_t zzenc_32(int32_t n) {
    return ((uint32_t)n << 1) ^ (n >> 31);
}

inline int32_t zzdec_32(uint32_t n) {
    return (n >> 1) ^ -(int32_t)(n & 1);
}

inline uint8_t zzenc_8(int8_t n) {
    return ((uint8_t)n << 1) ^ (n >> 7);
}

inline int8_t zzdec_8(uint8_t n) {
    return (n >> 1) ^ -(int8_t)(n & 1);
}
