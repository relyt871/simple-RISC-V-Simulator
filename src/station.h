#ifndef STATION_H
#define STATION_H

#include "tools.h"
#include "instructions.h"

struct RSInfo {
    instruction_t op;
    int qj, qk;
    uint vj, vk, A, rd;
    bool busy;
    RSInfo() {
        qj = qk = -1;
        busy = 1;
    }
};

class ReservationStation {

public:
    static const int SIZ = 30;
    RSInfo a[SIZ];

    ReservationStation() {
        for (int i = 0; i < SIZ; ++i) {
            a[i].busy = 0;
        }
    }

    void clear() {
        for (int i = 0; i < SIZ; ++i) {
            a[i].busy = 0;
        }
    }

    bool full() {
        for (int i = 0; i < SIZ; ++i) {
            if (!a[i].busy) {
                return 0;
            }
        }
        return 1;
    }

    int front() const {
        for (int i = 0; i < SIZ; ++i) {
            if (a[i].busy) {
                return i;
            }
        }
        return -1;
    }

    void push(const RSInfo &x) {
        for (int i = 0; i < SIZ; ++i) {
            if (!a[i].busy) {
                a[i] = x;
            }
        }
    }

    void update(int id, uint x) {
        for (int i = 0; i < SIZ; ++i) {
            if (a[i].busy) {
                if (a[i].qj == id) {
                    a[i].qj = -1;
                    a[i].vj = x;
                }
                if (a[i].qk == id) {
                    a[i].qk = -1;
                    a[i].vk = x;
                }
            }
            if (++i == QSIZ) i = 0;
        }
    }
};

#endif