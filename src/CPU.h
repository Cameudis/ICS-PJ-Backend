#ifndef CPU_H
#define CPU_H

#include "common.h"
#include "Register.h"
#include "Memory.h"
#include <fstream>
#include <vector>

#include "include/json.hpp"
using json = nlohmann::json;

enum State {
    SAOK = 1,    // Normal operation
    SHLT = 2,    // Halt instruction encountered
    SADR = 3,    // Invalid address encountered
    SINS = 4,    // Invalid instruction encountered
};

static const char* State_name[] = {
    "NULL",     // 0
    "AOK",      // 1
    "HLT",      // 2
    "ADR",      // 3
    "INS"       // 4
};

enum Ins {
    IHALT, INOP, IRRMOVQ, IIRMOVQ,
    IRMMOVQ, IMRMOVQ, IOPQ, IJXX,
    ICALL, IRET, IPUSHQ, IPOPQ,
    IIADDQ,
};  // P.S. SEQ use a function pointer table to code instructions

static const char* Ins_name[]{
    "IHALT", "INOP", "IRRMOVQ", "IIRMOVQ",
    "IRMMOVQ", "IMRMOVQ", "IOPQ", "IJXX",
    "ICALL", "IRET", "IPUSHQ", "IPOPQ",
    "IIADDQ",
};

enum ALUCode {
    ALUADD,
    ALUSUB,
    ALUAND,
    ALUXOR
};

struct Condition_code {
    bool ZF;
    bool OF;
    bool SF;
};

class CPU {
public:
    // --- init ---
    virtual void reset() = 0;
    virtual void load_prog(std::istream& infile) = 0;

    // --- exec ---

    // exec n steps, update (json)history and PC
    virtual void exec(unsigned int n) = 0;

    // back n steps, recover (json)history and all states
    virtual bool back(unsigned int n) = 0;

    // SP: exec an immediate instruction, keep PC, update (json)history
    virtual void im_exec(Instruction ins) = 0;

    // --- output ---
    virtual bool is_SAOK() const = 0;

    json history;
    virtual void update_history() = 0;
    virtual void print_history() = 0;
    virtual bool get_state(bool* cc, int* stat, _word_t* pc, _word_t* reg, int8_t* mem) = 0;
    virtual void get_PRstate(char* fbuf, char* dbuf, char* ebuf, char* mbuf, char* wbuf) = 0;
};

#endif