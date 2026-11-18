//
// NDN-IoT-RL - compact Bloom filter for inter-router cache digests
// (thesis 3.5.2: ~10-20 bytes/item, exchanged every 30-120 s).
// k hash positions derived from two independent string hashes
// (Kirsch-Mitzenmacher double hashing). Hex-string (de)serialization so the
// filter rides in InterestPacket.appPayload.
//

#ifndef __NDNIOT_BLOOMFILTER_H
#define __NDNIOT_BLOOMFILTER_H

#include <cstdint>
#include <algorithm>
#include <string>
#include <vector>

namespace ndniot {

class BloomFilter {
public:
    BloomFilter(uint32_t bits = 1024, uint32_t numHashes = 4)
        : m(bits), k(numHashes), data((bits + 7) / 8, 0) {}

    void add(const std::string &s) {
        uint32_t h1 = fnv(s), h2 = djb2(s);
        for (uint32_t i = 0; i < k; i++) {
            uint32_t pos = (h1 + i * h2) % m;
            data[pos >> 3] |= (1u << (pos & 7));
        }
    }

    // Saturate every bit: used only by the misreporting-neighbor arm, which
    // models a node advertising that it holds everything.
    void setAll() { std::fill(data.begin(), data.end(), (uint8_t)0xFF); }

    bool contains(const std::string &s) const {
        uint32_t h1 = fnv(s), h2 = djb2(s);
        for (uint32_t i = 0; i < k; i++) {
            uint32_t pos = (h1 + i * h2) % m;
            if (!(data[pos >> 3] & (1u << (pos & 7))))
                return false;
        }
        return true;
    }

    void clear() { std::fill(data.begin(), data.end(), 0); }
    size_t byteSize() const { return data.size(); }

    std::string toHex() const {
        static const char *d = "0123456789abcdef";
        std::string out;
        out.reserve(data.size() * 2);
        for (uint8_t b : data) {
            out.push_back(d[b >> 4]);
            out.push_back(d[b & 0xf]);
        }
        return out;
    }

    bool fromHex(const std::string &hex) {
        if (hex.size() != data.size() * 2)
            return false;
        auto nib = [](char c) -> int {
            if (c >= '0' && c <= '9') return c - '0';
            if (c >= 'a' && c <= 'f') return c - 'a' + 10;
            return -1;
        };
        for (size_t i = 0; i < data.size(); i++) {
            int hi = nib(hex[2 * i]), lo = nib(hex[2 * i + 1]);
            if (hi < 0 || lo < 0)
                return false;
            data[i] = (uint8_t)((hi << 4) | lo);
        }
        return true;
    }

private:
    uint32_t m, k;
    std::vector<uint8_t> data;

    static uint32_t fnv(const std::string &s) {
        uint32_t h = 2166136261u;
        for (char c : s) { h ^= (uint8_t)c; h *= 16777619u; }
        return h;
    }
    static uint32_t djb2(const std::string &s) {
        uint32_t h = 5381;
        for (char c : s) h = ((h << 5) + h) + (uint8_t)c;
        return h | 1;  // ensure odd step for double hashing
    }
};

} // namespace ndniot

#endif
