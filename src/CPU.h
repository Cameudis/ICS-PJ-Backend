#ifndef __CPU_H
#define __CPU_H

#include "common.h"
#include "Register.h"
#include "Memory.h"
#include <fstream>

#include "include/json.hpp"
using json = nlohmann::json;

enum State {
    AOK = 1,    // Normal operation
    HLT = 2,    // Halt instruction encountered
    ADR = 3,    // Invalid address encountered
    INS = 4,    // Invalid instruction encountered
};

struct Condition_code {
    bool ZF;
    bool OF;
    bool SF;
};

class CPU {
public:
    // --- init ---
    CPU();
    void load_prog(std::istream& infile);

    // --- exec ---

    // exec n steps, update (json)history and PC
    void exec(unsigned int n);

    // back n steps, recover (json)history and all states
    void back(unsigned int n);

    // SP: exec an immediate instruction, keep PC, update (json)history
    void im_exec(Instruction ins);

    // --- output ---
    json history;
    void update_history();

private:

    // --- CPU Composition ---

    // States
    Memory DMEM;
    word_t PC;
    State Stat;
    Condition_code CC;
    Register RG;
    
    // CND calculator (a part of ALU)
    bool calc_cnd(int icode);
    
    // maybe a part of CPU...? Whatever
    bool addr_check(word_t vaddr);
    
    // --- instruction ---
    typedef int (CPU::*InsPtr)(Instruction);

    // --- instruction handler ---

    // exec without update PC, return length of ins
    int exec_once(Instruction ins);

    // --- instruction implementation ---

    int ins_halt(Instruction ins);
    int ins_nop(Instruction ins);
    int ins_rrmov(Instruction ins);
    int ins_irmov(Instruction ins);
    int ins_rmmov(Instruction ins);
    int ins_mrmov(Instruction ins);
    int ins_op(Instruction ins);
    int ins_jmp(Instruction ins);
    int ins_call(Instruction ins);
    int ins_ret(Instruction ins);
    int ins_push(Instruction ins);
    int ins_pop(Instruction ins);
    int ins_null_handler(Instruction ins);

    // --- instruction encode (icode) ---

    InsPtr instab[0x10] = {
        ins_halt,           // 0x0
        ins_nop,            // 0x1
        ins_rrmov,          // 0x2
        ins_irmov,          // 0x3
        ins_rmmov,          // 0x4
        ins_mrmov,          // 0x5
        ins_op,             // 0x6
        ins_jmp,            // 0x7
        ins_call,           // 0x8
        ins_ret,            // 0x9
        ins_push,           // 0xa
        ins_pop,            // 0xb
        ins_null_handler,
        ins_null_handler,
        ins_null_handler,
        ins_null_handler,
    };

};

#endif