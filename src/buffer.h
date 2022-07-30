#ifndef BUFFER_H
#define BUFFER_H

const int QSIZ = 30;

template <typename T>
class Queue {
public:
    T que[QSIZ];
    int head, tail;

    Queue() {
        head = tail = 0;
    }

    bool empty() const {
        return head == tail;
    }

    bool full() const {
        return head == 0? (tail == QSIZ - 1) : (tail == head - 1);
    }

    void clear() {
        head = tail = 0;
    }

    void push(T &x) {
        que[tail] = x;
        if (++tail == QSIZ) tail = 0;
    }

    T front() const {
        return que[head];
    }

    void pop() {
        if (++head == QSIZ) head = 0;
    }
};

struct ROInfo {
    instruction_t op;
    function_t func;
    uint rd, val, pc;
    bool ready;
    int lsb_pos, rob_pos;
    ROInfo() {}
    ROInfo(instruction_t _op, function_t _func, uint _rd, uint _pc, bool _ready, int _lsb_pos, int _rob_pos):
        op(_op), func(_func), rd(_rd), pc(_pc), ready(_ready), lsb_pos(_lsb_pos), rob_pos(_rob_pos) {}
};

class ReorderBuffer : public Queue<ROInfo> {
public:
    ReorderBuffer() {
        Queue();
    }

    void update(int pos, uint val) {
        que[pos].val = val;
        que[pos].ready = 1;
    } 

    int apply() const {
        return tail;
    }
};

struct LSInfo {
    instruction_t op;
    function_t func;
    int qj, qk;
    uint vj, vk, A, rd;
    bool ready;
    LSInfo() {
        qj = qk = -1;
        ready = 0;
    }
};

class LoadStoreBuffer : public Queue<LSInfo> {
public:
    LoadStoreBuffer() {
        Queue();
    }

    void update(int id, uint x) {
        for (int i = head; i != tail; ) {
            if (que[i].qj == id) {
                que[i].qj = -1;
                que[i].vj = x;
            }
            if (que[i].qk == id) {
                que[i].qk = -1;
                que[i].vk = x;
            }
            if (++i == QSIZ) i = 0;
        }
    }

    int apply() const {
        return tail;
    }
};

#endif