#ifndef TOOLS_H
#define TOOLS_H

#include <iostream>
#include <cstring>

typedef long long LL;
typedef unsigned int uint;
typedef unsigned char uchar;

inline uint ExtractBit(uint x, int y) {
    return (x >> y) & 1;
}

inline uint ExtractBits(uint x, int l, int r) {
    return (x >> l) & ((1 << (r - l + 1)) - 1);
}

inline int SignExtend(uint x, int sign_bit) {
    if ((x >> (sign_bit - 1)) & 1) {
        return x | (0xFFFFFFFF >> sign_bit << sign_bit);
    } else {
        return x;
    }
}

inline uint Translate(char *s) {
    uint ret = 0;
    for (int i = 0, len = strlen(s); i < len; ++i) {
        if (isdigit(s[i])) {
            ret = (ret << 4) | (s[i] - '0');
        } else {
            ret = (ret << 4) | (10 + s[i] - 'A');
        }
    }
    return ret;
}

template <typename T1, typename T2>
class Pair {
public:
    T1 first;
    T2 second;
    Pair() {}
    Pair(const T1 &_first, const T2 &_second): 
        first(_first), second(_second) {}
};

#endif