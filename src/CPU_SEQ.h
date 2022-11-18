#ifndef __CPU_SEQ_H
#define __CPU_SEQ_H

#include "common.h"
#include "Register.h"
#include "Memory.h"
#include <fstream>

#include "include/json.hpp"
using json = nlohmann::json;

enum State {
    SAOK = 1,    // Normal operation
    SHLT = 2,    // Halt instruction encountered
    SADR = 3,    // Invalid address encountered
    SINS = 4,    // Invalid instruction encountered
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
    void reset();
    void load_prog(std::istream& infile);

    // --- exec ---

    // exec n steps, update (json)history and PC
    void exec(unsigned int n);

    // back n steps, recover (json)history and all states
    bool back(unsigned int n);

    // SP: exec an immediate instruction, keep PC, update (json)history
    void im_exec(Instruction ins);

    // --- output ---
    json history;
    void update_history();

private:

    // --- CPU Composition ---

    // States
    Memory DMEM;
    _word_t PC;
    State Stat;
    Condition_code CC;
    Register RG;
    
    // CND calculator (a part of ALU)
    bool calc_cnd(int icode);
    
    // maybe a part of CPU...? Whatever
    bool addr_check(_word_t vaddr);

    // --- instruction handler ---
     
    // --- instruction ---
    typedef int (CPU::* InsPtr)(Instruction);

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
    int ins_iadd(Instruction ins);
    int ins_null_handler(Instruction ins);

    // --- instruction encode (icode) ---

    InsPtr instab[0x10] = {
        &CPU::ins_halt,         // 0x0
        &CPU::ins_nop,          // 0x1
        &CPU::ins_rrmov,        // 0x2
        &CPU::ins_irmov,        // 0x3
        &CPU::ins_rmmov,        // 0x4
        &CPU::ins_mrmov,        // 0x5
        &CPU::ins_op,           // 0x6
        &CPU::ins_jmp,          // 0x7
        &CPU::ins_call,         // 0x8
        &CPU::ins_ret,          // 0x9
        &CPU::ins_push,         // 0xa
        &CPU::ins_pop,          // 0xb
        &CPU::ins_iadd,         // 0xc
        &CPU::ins_null_handler,
        &CPU::ins_null_handler,
        &CPU::ins_null_handler,
    };

};

#endif