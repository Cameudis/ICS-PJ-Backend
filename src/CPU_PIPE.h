#ifndef __CPU_PIPE_H
#define __CPU_PIPE_H

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

    // exec without update PC, return length of ins
    int exec_once(Instruction ins);

};

#endif