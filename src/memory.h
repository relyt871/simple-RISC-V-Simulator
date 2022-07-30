#ifndef MEMORY_H
#define MEMORY_H

#include "tools.h"

class Memory {

private:
    uint mem[500005];

public:
    uint & operator [] (const int &pos) {
        return mem[pos];
    }

    inline uint Read(int pc, int len) {
        uint ret = 0;
        for (int i = 0; i < len; ++i) {
            ret |= (mem[pc + i] << (i << 3));
        }
        return ret;
    }

    inline void Write(int pc, int len, uint val) {
        for (int i = 0; i < len; ++i) {
            mem[pc + i] = (val & 0xFF);
            val >>= 8;
        }
    }
};

#endif