#ifndef __CPU_SEQ_H
#define __CPU_SEQ_H

#include "CPU.h"

class CPU_SEQ : public CPU {
public:
    // --- init ---
    CPU_SEQ();
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
    bool is_SAOK() const { return (history[history.size() - 1]["STAT"]) == SAOK; }

    json history;
    void update_history();
    void print_history();
    bool get_state(bool *cc, int *stat, _word_t *pc, _word_t *reg, int8_t *mem);

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
    typedef int (CPU_SEQ::* InsPtr)(Instruction);

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
        &CPU_SEQ::ins_halt,         // 0x0
        &CPU_SEQ::ins_nop,          // 0x1
        &CPU_SEQ::ins_rrmov,        // 0x2
        &CPU_SEQ::ins_irmov,        // 0x3
        &CPU_SEQ::ins_rmmov,        // 0x4
        &CPU_SEQ::ins_mrmov,        // 0x5
        &CPU_SEQ::ins_op,           // 0x6
        &CPU_SEQ::ins_jmp,          // 0x7
        &CPU_SEQ::ins_call,         // 0x8
        &CPU_SEQ::ins_ret,          // 0x9
        &CPU_SEQ::ins_push,         // 0xa
        &CPU_SEQ::ins_pop,          // 0xb
        &CPU_SEQ::ins_iadd,         // 0xc
        &CPU_SEQ::ins_null_handler,
        &CPU_SEQ::ins_null_handler,
        &CPU_SEQ::ins_null_handler,
    };

};

#endif