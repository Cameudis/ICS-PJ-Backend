#include "common.h"
#include "Register.h"
#include "Memory.h"
#include <fstream>

enum State {
    AOK = 1,    // Normal operation
    HLT = 2,    // Halt instruction encountered
    ADR = 3,    // Invalid address encountered
    INS = 4,    // Invalid instruction encountered
};

const char* State_name[] = {
    "NULL",     // 0
    "AOK",      // 1
    "HLT",      // 2
    "ADR",      // 3
    "INS"       // 4
};

struct Condition_code {
    bool ZF;
    bool OF;
    bool SF;
};

class CPU {
public:
    // init
    CPU();
    void load_prog(std::ifstream& infile);

    // exec
    void exec(int n);
    void back(int n);
    void im_exec(Instruction ins);      // exec an immediate instruction

    // output
    // TODO

private:
    // exec without update PC
    // return length of ins
    int exec_once(Instruction ins);

    // instructions
    typedef int (CPU::*InsPtr)(Instruction);

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

    // numbering instructions
    // you can modify ins's ID by simply change the order here
    InsPtr instab[0xf] = {
        ins_halt,       // 0x0
        ins_nop,        // 0x1
        ins_rrmov,      // 0x2
        ins_irmov,      // 0x3
        ins_rmmov,      // 0x4
        ins_mrmov,      // 0x5
        ins_op,         // 0x6
        ins_jmp,        // 0x7
        ins_call,       // 0x8
        ins_ret,        // 0x9
        ins_push,       // 0xa
        ins_pop,        // 0xb
        ins_null_handler,
        ins_null_handler,
        ins_null_handler,
        ins_null_handler,
    };

    // CC judge
    bool ccjudge(int icode);

    // CPU Composition
    Memory DMEM;
    word_t PC;
    State Stat;
    Condition_code CC;
    Register RG;
};