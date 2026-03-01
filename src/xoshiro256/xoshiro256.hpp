#pragma once
#include <cstdint>

namespace xoshiro256 {

    struct xoshiro256starstar_state {
        uint64_t s[4];
    };

    inline uint64_t splitmix64(uint64_t& state) {
        uint64_t z = (state += 0x9e3779b97f4a7c15ULL);
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
        z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
        return z ^ (z >> 31);
    }

    inline uint64_t rotl(const uint64_t x, int k) {
        return (x << k) | (x >> (64 - k));
    }
    
    inline uint64_t next_u64(xoshiro256starstar_state& state) {
        const uint64_t result = rotl(state.s[1] * 5, 7) * 9;
        const uint64_t t = state.s[1] << 17;

        state.s[2] ^= state.s[0];
        state.s[3] ^= state.s[1];
        state.s[1] ^= state.s[2];
        state.s[0] ^= state.s[3];
        state.s[2] ^= t;
        state.s[3] = rotl(state.s[3], 45);

        return result;
    }

    inline double next_double(xoshiro256starstar_state& state) {
        uint64_t v = next_u64(state);
        return (v >> 11) * 0x1.0p-53; 
    }
}