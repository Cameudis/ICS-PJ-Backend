#ifndef __CPU_PIPE_H
#define __CPU_PIPE_H

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

enum Ins {
    IHALT,      INOP,       IRRMOVQ,    IIRMOVQ,
    IRMMOVQ,    IMRMOVQ,    IOPQ,       IJXX,
    ICALL,      IRET,       IPUSHQ,     IPOPQ,
};

enum ALUCode
{
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
    CPU();
    void reset();
    void load_prog(std::istream& infile);

    // --- exec ---

    // exec n steps, update (json)history and PC
    void exec(unsigned int n);

    // --- output ---
    json history;
    std::vector<bool> history_valid;    // for bubble
    void update_history();
    void print_history();
    void debug();

private:

    // --- CPU Composition ---

    // Programmer-visible States

    Memory DMEM;
    // _word_t PC;
    State Stat;
    Condition_code CCnext;  // for m_stat check
    Condition_code CC;
    Register RG;

    // Pipeline Register

    struct Reg_Fetch {
        _word_t predPC;

        // pipeline control
        bool stall;
    } F, Fnext;

    struct Reg_Decode {
        State stat;
        int icode;
        int ifun;
        int rA;
        int rB;
        _word_t valC;
        _word_t valP;

        // pipeline control
        bool stall;
        bool bubble;

        // record history
        int history_ID;
        _word_t ins_addr;
    } D, Dnext;

    struct Reg_Execute {
        State stat;
        int icode;
        int ifun;
        _word_t valC;
        _word_t valA;
        _word_t valB;
        int dstE;
        int dstM;
        int srcA;
        int srcB;

        // pipeline control
        bool bubble;

        // record history
        int history_ID;
        _word_t ins_addr;
    } E, Enext;

    struct Reg_Memory {
        State stat;
        int icode;
        bool Cnd;
        _word_t valE;
        _word_t valA;
        int dstE;
        int dstM;

        // pipeline control
        bool bubble;

        // record history
        int history_ID;
        _word_t ins_addr;
    } M, Mnext;

    struct Reg_Writeback {
        State stat;
        int icode;
        _word_t valE;
        _word_t valM;
        int dstE;
        int dstM;

        // pipeline control
        bool bubble;

        // record history
        int history_ID;
        _word_t ins_addr;
    } W, Wnext;
    
    // CND calculator (a part of ALU)
    bool calc_cnd(int icode);

    // --- Execute ---

    // exec without update PC, return length of ins
    bool exec_once();

    void fetch();
    void decode();
    void execute();
    void memoryAccess();
    void writeBack();

};

#endif