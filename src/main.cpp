#include <iostream>
#include "tomasulo.h"

//#define LOCAL

int main() {

#ifdef LOCAL
    freopen("testcases/bulgarian.data", "r", stdin);
#endif

    Tomasulo_Simulator s;
    s.input();
    s.run();
    return 0;
}