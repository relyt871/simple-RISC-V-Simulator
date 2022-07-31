#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include "tools.h"
#include <iostream>

enum instruction_t {
    LUI,      //Load Upper Immediate
    AUIPC,    //Add Upper Immediate to PC
    JAL,      //Jump And Link
    JALR,     //Jump and Link Register
    BEQ,      //Branch if Equal
    BNE,      //Branch if Not Equal
    BLT,      //Branch if Less Than
    BGE,      //Branch if Greater Than or Equal
    BLTU,     //Branch if Less Than, Unsigned
    BGEU,     //Branch if Greater Than or Equal, Unsigned
    LB,       //Load Byte
    LH,       //Load Halfword
    LW,       //Load Word
    LD,       //Load Doubleword
    LBU,      //Load Byte, Unsigned
    LHU,      //Load Halfword, Unsigned
    LWU,      //Load Word, Unsigned
    SB,       //Store Byte
    SH,       //Store Halfword
    SW,       //Store Word
    SD,       //Store Doubleword
    ADDI,     //Add Immediate
    SLTI,     //Set if Less Than Immediate
    SLTIU,    //Set if Less Than Immediate, Unsigned
    XORI,
    ORI,
    ANDI,
    SLLI,     //Shift Left Logical Immediate
    SRLI,     //Shift Right Logical Immediate
    SRAI,     //Shift Right Arithmetic Immediate
    ADD,
    SUB,
    SLL,
    SLT,
    SLTU,
    XOR,
    SRL,
    SRA,
    OR,
    AND,
    HALT,
    WOW
};

enum function_t {
    IMM, JUMP, BRANCH, LOAD, STORE, CALCI, CALC, RET
};

const instruction_t Btype[8] = {BEQ, BNE, WOW, WOW, BLT, BGE, BLTU, BGEU};
const instruction_t Itype1[8] = {LB, LH, LW, LD, LBU, LHU};
const instruction_t Stype[8] = {SB, SH, SW, SD};
const instruction_t Itype2[8] = {ADDI, SLLI, SLTI, SLTIU, XORI, SRLI, ORI, ANDI};
const instruction_t Rtype[8] = {ADD, SLL, SLT, SLTU, XOR, SRL, OR, AND};

struct Instruction {
    instruction_t TYPE;
    function_t FTYPE;
    uint rd, rs1, rs2, imm, pc, pred_pc;
    Instruction() {}
    Instruction(instruction_t _TYPE, uint _rd, uint _rs1, uint _rs2, uint _imm):
        TYPE(_TYPE), rd(_rd), rs1(_rs1), rs2(_rs2), imm(_imm) {}
};

uint tmp;

Instruction Decode(uint ins) {
    Instruction cur;
    if (ins == 0x0ff00513) {
        cur.TYPE = HALT;
		cur.FTYPE = RET;
        return cur;
    }
    uint opcode = ExtractBits(ins, 0, 6), func3;
    switch(opcode) {
    case 0x37:
        cur.TYPE = LUI;
        cur.FTYPE = IMM;
        cur.rd = ExtractBits(ins, 7, 11);
        cur.imm = ExtractBits(ins, 12, 31) << 12;
        break;
    case 0x17:
        cur.TYPE = AUIPC;
        cur.FTYPE = IMM;
        cur.rd = ExtractBits(ins, 7, 11);
        cur.imm = ExtractBits(ins, 12, 31) << 12;
        break;
    case 0x6F:
        cur.TYPE = JAL;
        cur.FTYPE = JUMP;
        cur.rd = ExtractBits(ins, 7, 11);
        tmp = (ExtractBit(ins, 31) << 20) | (ExtractBits(ins, 21, 30) << 1)
                 | (ExtractBit(ins, 20) << 11) | (ExtractBits(ins, 12, 19) << 12);
        cur.imm = SignExtend(tmp, 21);
        break;
    case 0x67:
        cur.TYPE = JALR;
        cur.FTYPE = JUMP;
        cur.rd = ExtractBits(ins, 7, 11);
        cur.rs1 = ExtractBits(ins, 15, 19);
        cur.imm = SignExtend(ExtractBits(ins, 20, 31), 12);
        break;
    case 0x63:
        func3 = ExtractBits(ins, 12, 14);
        cur.TYPE = Btype[func3];
        cur.FTYPE = BRANCH;
        cur.rs1 = ExtractBits(ins, 15, 19);
        cur.rs2 = ExtractBits(ins, 20, 24);
        tmp = (ExtractBit(ins, 31) << 12) | (ExtractBits(ins, 25, 30) << 5)
                 | (ExtractBit(ins,  7) << 11) | (ExtractBits(ins, 8, 11) << 1);
        cur.imm = SignExtend(tmp, 13);
        break;
    case 0x03:
        func3 = ExtractBits(ins, 12, 14);
        cur.TYPE = Itype1[func3];
        cur.FTYPE = LOAD;
        cur.rd = ExtractBits(ins, 7, 11);
        cur.rs1 = ExtractBits(ins, 15, 19);
        cur.imm = SignExtend(ExtractBits(ins, 20, 31), 12);
        break;
    case 0x23:
        func3 = ExtractBits(ins, 12, 14);
        cur.TYPE = Stype[func3];
        cur.FTYPE = STORE;
        cur.rs1 = ExtractBits(ins, 15, 19);
        cur.rs2 = ExtractBits(ins, 20, 24);
        tmp = (ExtractBits(ins, 25, 31) << 5) | ExtractBits(ins, 7, 11);
        cur.imm = SignExtend(tmp, 12);
        break;
    case 0x13:
        func3 = ExtractBits(ins, 12, 14);
        cur.TYPE = Itype2[func3];
        cur.FTYPE = CALCI;
        if (func3 == 5) {
            if (ExtractBit(ins, 30)) {
                cur.TYPE = SRAI;
            }
        }
        cur.rd = ExtractBits(ins, 7, 11);
        cur.rs1 = ExtractBits(ins, 15, 19);
        cur.imm = SignExtend(ExtractBits(ins, 20, 31), 12);
        break;
    case 0x33:
        func3 = ExtractBits(ins, 12, 14);
        cur.TYPE = Rtype[func3];
        cur.FTYPE = CALC;
        if (func3 == 0) {
            if (ExtractBit(ins, 30)) {
                cur.TYPE = SUB;
            }
        }
        if (func3 == 5) {
            if (ExtractBit(ins, 30)) {
                cur.TYPE = SRA;
            }
        }
        cur.rd = ExtractBits(ins, 7, 11);
        cur.rs1 = ExtractBits(ins, 15, 19);
        cur.rs2 = ExtractBits(ins, 20, 24);
        break;
    default:
        cur.TYPE = WOW;
//std::cerr << "WOW" << std::endl;
        break;
    }
    return cur;
}

#endif