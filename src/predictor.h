#ifndef PREDICTOR_H
#define PREDICTOR_H

#include "tools.h"

class BranchPredictor {
public:
    BranchPredictor() {
        for (int i = 0; i < 4096; ++i) {
            cnt[i] = 1;
            res[i] = 0;
        }
    }

    bool res[4096];
    uchar cnt[4096];

    void update(uint pc, bool x) {
        pc &= 4095;
        if (x == res[pc]) {
            if (cnt[pc] < 4) ++cnt[pc];
        } else {
            if (cnt[pc] > 1) {
                --cnt[pc];
            } else {
                res[pc] ^= 1;
            }
        }
    }

    bool predict(uint pc) {
        return res[pc & 4095];
    }
};

#endif