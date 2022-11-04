#include "common.h"
#include "Register.h"
#include "Memory.h"
#include <fstream>

enum state {
    AOK = 1,    // Normal operation
    HLT = 2,    // Halt instruction encountered
    ADR = 3,    // Invalid address encountered
    INS = 4,    // Invalid instruction encountered
};

struct condition_code {
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

private:
    Memory DMEM;
    uint64_t PC;
    state Stat;
    condition_code CC;
    Register RG;
};