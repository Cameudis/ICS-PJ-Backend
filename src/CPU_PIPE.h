#ifndef __CPU_PIPE_H
#define __CPU_PIPE_H

#include "CPU.h"

class CPU_PIPE: public CPU {
public:
    // --- init ---
    CPU_PIPE();
    void reset();
    void load_prog(std::istream& infile);

    // --- exec ---

    // exec n steps, update (json)history and PC
    void exec(unsigned int n);

    // unsupported
    bool back(unsigned int n) { return false; };
    void im_exec(Instruction ins) {};

    // --- output ---
    bool is_SAOK() const { return Stat == SAOK; }

    json history;
    std::vector<bool> history_valid;    // for bubble

    void update_history();
    void print_history();
    void debug();

    bool get_state(bool* cc, int* stat, _word_t* pc, _word_t* reg, int8_t* mem);
    void get_PRstate(char* fbuf, char* dbuf, char* ebuf, char* mbuf, char* wbuf);

private:

    void create_record(_word_t iaddr);

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
        bool bp_taken;

        // record history
        int history_ID;
        _word_t ins_addr;
    } D, Dnext;

    struct Reg_Execute {
        State stat;
        int icode;
        int ifun;
        _word_t valC;
        _word_t valA;   // E_valA Enext_valA
        _word_t valB;   // E_valB Enext_valB
        int dstE;
        int dstM;
        int srcA;
        int srcB;

        // pipeline control
        bool bubble;
        bool bp_taken;

        // record history
        int history_ID;
        _word_t ins_addr;
    } E, Enext;

    struct Reg_Memory {
        State stat;
        int icode;
        int ifun;
        bool Cnd;
        _word_t valE;
        _word_t valA;
        int dstE;
        int dstM;

        // pipeline control
        bool bubble;
        bool bp_taken;

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

    // Branch Predict (bimodal branch prediction)
    enum {
        Never_Taken,
        Always_Taken,
        Branch_Predict_Bimodal,
    } strategy;
    enum PredictorState {
        WEAK_TAKEN,         // default
        STRONG_TAKEN,
        WEAK_NOT_TAKEN,
        STRONG_NOT_TAKEN,
    } bp_buffer[PRED_BUF_SIZE];

    bool branch_predict(_word_t pc);
    void branch_update(_word_t pc, bool branch);

#ifdef BPTEST
    unsigned int correct_pred;
    unsigned int mispred;
#endif

    // --- Execute ---

    bool exec_once();

    void fetch();
    void decode();
    void execute();
    void memoryAccess();
    void writeBack();

};

#endif