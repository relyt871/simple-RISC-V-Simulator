#ifndef TOMASULO_H
#define TOMASULO_H

#include "tools.h"
#include "instructions.h"
#include "memory.h"
#include "buffer.h"
#include "station.h"

#include <iostream>
#include <vector>
using std::vector;

class Tomasulo_Simulator {
private:
    uint reg[32], PC;
    Memory mem;

    struct RegInfo {
        bool busy;
        int qi;
        uint val;
        RegInfo() {
            busy = 0;
            qi = -1;
        }
    };

    struct All {
        RegInfo regfile[32];
        Queue<Instruction> insq;
        ReorderBuffer robuffer;
        LoadStoreBuffer lsbuffer;
        ReservationStation rstation;
        vector< Pair<int, uint> > cdb;
    } pre, cur;

    vector<ROInfo> newro, can_commit;
    vector<LSInfo> newls;
    vector<RSInfo> newrs, can_exe;
    vector< Pair<int, int> > rf_lock, rf_unlock;


    void Update() {
        pre = cur;
        cur.cdb.clear();
        newro.clear();
        newls.clear();
        newrs.clear();
        rf_lock.clear();
        rf_unlock.clear();
    }

    void RunROBuffer() {
        for (auto x : newro) {
            cur.robuffer.push(x);
        }
        for (auto x : pre.cdb) {
            cur.robuffer.update(x.first, x.second);
        }
        if (!cur.robuffer.empty()) {
            ROInfo u = cur.robuffer.front();
//std::cerr << "robuffer front " << u.pc << std::endl;
            if (u.ready) {
                can_commit.push_back(u);
                cur.robuffer.pop();
            }
        }
    }

    void RunLSBuffer() {
        for (auto x : newls) {
            cur.lsbuffer.push(x);
        }
        if (!cur.lsbuffer.empty()) {
            LSInfo u = cur.lsbuffer.front();
            if (u.qj == -1 && u.qk == -1) {
                if (u.func == LOAD) {
                    uint loadval;
                    switch (u.op) {
                    case LB:
                        loadval = SignExtend(mem.Read(u.vj + u.A, 1), 8);
                        break;
                    case LH:
                        loadval = SignExtend(mem.Read(u.vj + u.A, 2), 16);
                        break;
                    case LW:
                        loadval = SignExtend(mem.Read(u.vj + u.A, 4), 32);
                        break;
                    case LBU:
                        loadval = mem.Read(u.vj + u.A, 1);
                        break;
                    case LHU:
                        loadval = mem.Read(u.vj + u.A, 2);
                        break;
                    default:
                        break;
                    }
                    cur.lsbuffer.pop();
                    cur.cdb.push_back(Pair<int, uint>(u.rd, loadval));
                } else if (u.ready) {
                    switch (u.op) {
                    case SB:
                        mem.Write(u.vj + u.A, 1, u.vk);
                        break;
                    case SH:
                        mem.Write(u.vj + u.A, 2, u.vk);
                        break;
                    case SW:
                        mem.Write(u.vj + u.A, 4, u.vk);
                        break;
                    default:
                        break;
                    }
                    cur.lsbuffer.pop();
                } else {
                    cur.robuffer.update(u.rd, 0);
                }
            }
        }
        for (auto x : pre.cdb) {
            cur.lsbuffer.update(x.first, x.second);
        }
    }

    void RunReservation() {
        for (auto x : newrs) {
            cur.rstation.push(x);
        }
        int p = cur.rstation.front();
        if (p != -1) {
            RSInfo u = cur.rstation.a[p];
            if (u.qj == -1 && u.qk == -1) {
                can_exe.push_back(u);
                cur.rstation.a[p].busy = 0;
            }
        }
        for (auto x : pre.cdb) {
            cur.rstation.update(x.first, x.second);
        }
    }

    void RunRegfile() {
        static RegInfo* tmp;
        for (auto x : rf_lock) {
            tmp = &cur.regfile[x.first];
            tmp -> qi = x.second;
            tmp -> busy = 1;
        }
        for (auto x : rf_unlock) {
            tmp = &cur.regfile[x.first];
            if (tmp -> qi == x.second) {
                tmp -> qi = -1;
                tmp -> busy = 0;
            }
        }
    }

    void RunFetch() {
        if (cur.insq.full()) return;
        Instruction ins = Decode(mem.Read(PC, 4));
//std::cerr << std::hex << "fetch " << PC << ' ' << ins.TYPE << std::endl;
        if (ins.TYPE == WOW) return;
        ins.pc = PC;
        cur.insq.push(ins);
        PC += 4;
    }

    void RunExecute() {
        for (auto ins : can_exe) {
            uint val;
            switch (ins.op) {
            //imm only
            case LUI:
                val = ins.A;
                break;
            case AUIPC:
                val = ins.vj + ins.A;
                break;

            //branch
            case JAL:
                val = ins.A;
                break;
            case JALR:
                val = ((ins.vj + ins.A) & (-1));
                break;
            case BEQ:
                val = (ins.vj == ins.vk? ins.A : 4);
                break;
            case BNE:
                val = (ins.vj != ins.vk? ins.A : 4);
                break;
            case BLT:
                val = ((int)(ins.vj) < (int)(ins.vk)? ins.A : 4);
                break;
            case BGE:
                val = ((int)(ins.vj) >= (int)(ins.vk)? ins.A : 4);
                break;
            case BLTU:
                val = (ins.vj < ins.vk? ins.A : 4);
                break;
            case BGEU:
                val = (ins.vj >= ins.vk? ins.A : 4);
                break;

            //calculation with imm
            case ADDI:
                val = ins.vj + ins.A;
                break;
            case SLTI:
                val = ((int)ins.vj < (int)ins.A);
                break;
            case SLTIU:
                val = (ins.vj < ins.A);
                break;
            case XORI:
                val = ins.vj ^ ins.A;
                break;
            case ORI:
                val = ins.vj | ins.A;
                break;
            case ANDI:
                val = ins.vj & ins.A;
                break;
            case SLLI:
                val = ins.vj << ins.A;
                break;
            case SRLI:
                val = ins.vj >> ins.A;
                break;
            case SRAI:
                val = SignExtend(ins.vj >> ins.A, 32 - ins.A);
                break;
            //calculation
            case ADD:
                val = ins.vj + ins.vk;
                break;
            case SUB:
                val = ins.vj - ins.vk;
                break;
            case SLL:
                val = ins.vj << ins.vk;
                break;
            case SLT:
                val = ((int)(ins.vj) < (int)(ins.vk));
                break;
            case SLTU:
                val = (ins.vj < ins.vk);
                break;
            case XOR:
                val = ins.vj ^ ins.vk;
                break;
            case SRL:
                val = ins.vj >> ins.vk;
                break;
            case SRA:
                val = SignExtend(ins.vj >> ins.vk, 32 - ins.vk);
                break;
            case OR:
                val = ins.vj | ins.vk;
                break;
            case AND:
                val = ins.vj & ins.vk;
                break;
            }
            pre.cdb.push_back(Pair<int, uint>(ins.rd, val));
        }
        can_exe.clear();
    }

    inline Pair<int, uint> Get_rs(uint pos) {
        static RegInfo* tmp1;
        static ROInfo* tmp2;
        tmp1 = &pre.regfile[pos];
        if (tmp1 -> busy) {
            int where = tmp1 -> qi;
            tmp2 = &pre.robuffer.que[where];
            if (tmp2 -> ready) {
                return Pair<int, uint>(1, tmp2 -> val);
            } else {
                return Pair<int, uint>(0, where);
            }
        } else {
            return Pair<int, uint>(1, reg[pos]);
        }
    } 

    void RunIssue() {
        if (cur.insq.empty() || cur.robuffer.full() || cur.rstation.full()) {
            return;
        }
        Instruction ins = cur.insq.front();
        int pos = cur.robuffer.apply(), lsb_pos;

        static Pair<int, uint> tmp;

        if (ins.FTYPE == LOAD || ins.FTYPE == STORE) {
            if (cur.lsbuffer.full()) {
                return;
            }
            cur.insq.pop();
            lsb_pos = cur.lsbuffer.apply();
            LSInfo u;
            u.op = ins.TYPE;
            u.func = ins.FTYPE;
            u.rd = pos;
            u.A = ins.imm;
            tmp = Get_rs(ins.rs1);
            tmp.first? (u.vj = tmp.second) : (u.qj = tmp.second);
            if (ins.FTYPE == STORE) {
                tmp = Get_rs(ins.rs2);
                tmp.first? (u.vk = tmp.second) : (u.qk = tmp.second);
            }
            newls.push_back(u);
        } else if (ins.TYPE != HALT) {
            cur.insq.pop();
            RSInfo u;
            u.op = ins.TYPE;
            u.rd = pos;
            u.A = ins.imm;
            if (ins.TYPE == AUIPC) {
                u.vj = ins.pc;
            }
            if (ins.FTYPE != IMM && ins.TYPE != JAL) {
                tmp = Get_rs(ins.rs1);
                tmp.first? (u.vj = tmp.second) : (u.qj = tmp.second);
            }
            if (ins.FTYPE == BRANCH || ins.FTYPE == CALC) {
                tmp = Get_rs(ins.rs2);
                tmp.first? (u.vk = tmp.second) : (u.qk = tmp.second);
            }
            newrs.push_back(u);
        }

        ROInfo u(ins.TYPE, ins.FTYPE, ins.rd, ins.pc, (ins.TYPE == HALT), lsb_pos, pos);
        newro.push_back(u);

        if (ins.FTYPE != BRANCH && ins.FTYPE != STORE && ins.TYPE != HALT) {
            rf_lock.push_back(Pair<int, int>(ins.rd, pos));
        }
    }

    void RollBack() {
        for (int i = 0; i < 32; ++i) {
            cur.regfile[i].busy = 0;
            cur.regfile[i].qi = -1;
        }
        cur.insq.clear();
        cur.robuffer.clear();
        cur.lsbuffer.clear();
        cur.rstation.clear();
        cur.cdb.clear();
        newro.clear();
        newls.clear();
        newrs.clear();
        can_exe.clear();
        rf_lock.clear();
        rf_unlock.clear();
    }

    bool RunCommit() {
        for (auto x : can_commit) {
//std::cerr << "commit " << x.pc << ' ' << x.op << std::endl; 
            if (x.op == HALT) {
                return 0;
            }
            if (x.func == JUMP || x.func == BRANCH) {
                if (x.func == JUMP) {
                    reg[x.rd] = x.pc + 4;
                    rf_unlock.push_back(Pair<int, int>(x.rd, x.rob_pos));
                }
                if (x.op == JALR) {
                    if (x.val != x.pc + 4) {
                        RollBack();
                        PC = x.val;
                    }
                } else {
                    if (x.val != 4) {
                        RollBack();
                        PC = x.pc + x.val;
//std::cerr << "branch " << x.val << ' ' << PC << std::endl;
                    }
                }
            } else if (x.func == STORE) {
                cur.lsbuffer.que[x.lsb_pos].ready = 1;
            } else {
                reg[x.rd] = x.val;
                rf_unlock.push_back(Pair<int, int>(x.rd, x.rob_pos));
            }
        }
       can_commit.clear();
       reg[0] = 0;
       return 1;
    }

public:
    void input() {
        char s[100];
        int ptr;
        while (scanf("%s", s) != EOF) {
            if (s[0] == '@') {
                ptr = Translate(s + 1);
            } else {
                mem[ptr++] = Translate(s);
            }
        }
    }

    void run() {
        PC = 0;
        uint clk = 0;
        while (871) {
            ++clk;
//std::cerr << "clk  " << clk << std::endl;
            RunROBuffer();
            RunLSBuffer();
            RunReservation();
            RunRegfile();
            RunFetch();
            Update();
            RunExecute();
            RunIssue();
            if (!RunCommit()) {
                break;
            }
        }
        printf("%u\n", reg[10] & 255u);
        //printf("%u\n", clk);
    }
};

#endif