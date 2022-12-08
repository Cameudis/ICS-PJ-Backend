#include "CPU_PIPE.h"
#include <cstring>
#include <cstdio>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include  <algorithm>
using std::map; using std::string; using std::vector;

static const char* State_name[] = {
    "NULL",     // 0
    "AOK",      // 1
    "HLT",      // 2
    "ADR",      // 3
    "INS"       // 4
};

// ---------- init ----------

CPU_PIPE::CPU_PIPE()
{
    reset();
}

void CPU_PIPE::reset()
{
    CCnext.ZF = CC.ZF = 1;
    CCnext.OF = CC.OF = 0;
    CCnext.SF = CC.SF = 0;
    Stat = SAOK;
    DMEM.clear();
    RG.clear();

    memset(&F, 0, sizeof(F));
    memset(&Fnext, 0, sizeof(Fnext));
    memset(&D, 0, sizeof(D));
    memset(&Dnext, 0, sizeof(Dnext));
    memset(&E, 0, sizeof(E));
    memset(&Enext, 0, sizeof(Enext));
    memset(&M, 0, sizeof(M));
    memset(&Mnext, 0, sizeof(Mnext));
    memset(&W, 0, sizeof(W));
    memset(&Wnext, 0, sizeof(Wnext));
    D.stat = SAOK;
    Dnext.stat = SAOK;
    E.stat = SAOK;
    Enext.stat = SAOK;
    M.stat = SAOK;
    Mnext.stat = SAOK;
    W.stat = SAOK;
    Wnext.stat = SAOK;

    memset(&bp_buffer, 0, sizeof(bp_buffer));
    strategy = Branch_Predict_Bimodal;

    history.clear();
    history_valid.clear();
}

void CPU_PIPE::load_prog(std::istream& infile)
{
    char str[100];

    char byte_s[3] = "00";
    uint8_t byte;

    reset();

    while (!infile.eof()) {
        // read instruction
        infile.getline(str, 100, '|');

        // fprintf(stderr, "\n'%s'", str);

        char* ins_str = strchr(str, ':');
        if (ins_str) {
            _word_t vaddr = 0;
            sscanf(str, "%llx", &vaddr);

            // fprintf(stderr, "0x%03llx: ", vaddr);

            ins_str += 2;   // jmp ": "

            while (isalnum(*ins_str)) {
                byte_s[0] = ins_str[0];
                byte_s[1] = ins_str[1];
                byte_s[2] = '\0';
                // fputs(byte_s, stderr);
                sscanf(byte_s, "%hhx", &byte);
                *DMEM.v2raddr(vaddr++) = byte;

                ins_str += 2;
            }
        }
        infile.getline(str, 100);
    }
}

// ---------- output ----------

void CPU_PIPE::create_record(_word_t iaddr)
{
    json record;
    record["PC"] = iaddr;    // temp val, will be update by next AOK instruction(not bubble, not s)
    for (int i = 0; i < 15; i++) {
        record["REG"][RG.get_reg_name(i)] = 0;
    }
    record["STAT"] = SAOK;
    record["CC"]["OF"] = 0;
    record["CC"]["SF"] = 0;
    record["CC"]["ZF"] = 0;
    record["MEM"]["0"] = 0;
    history.push_back(record);
    history_valid.push_back(true);
}

void CPU_PIPE::update_history()
{
    int crt_done = W.history_ID;
    if (crt_done >= 0) {
        // PC dump
        if (!Wnext.bubble && W.stat == SAOK) {
            history[crt_done]["PC"] = Wnext.ins_addr;
        }
        // REG dump
        for (int i = 0; i < 15; i++) {
            history[crt_done]["REG"][RG.get_reg_name(i)] = (int64_t)RG[i];
        }

        // STAT dump
        history[crt_done]["STAT"] = W.stat;
    }

    // CC dump
    if (E.history_ID >= 0) {
        history[E.history_ID]["CC"]["OF"] = (int)CCnext.OF;
        history[E.history_ID]["CC"]["SF"] = (int)CCnext.SF;
        history[E.history_ID]["CC"]["ZF"] = (int)CCnext.ZF;
    }


    // MEM dump
    if (M.history_ID >= 0) {
        for (_word_t vaddr = 0; vaddr < MSIZE; vaddr += 8) {
            if (DMEM[vaddr]) {
                char vaddr_str[10];
                sprintf(vaddr_str, "%lld", vaddr);

                history[M.history_ID]["MEM"][vaddr_str] = (int64_t)DMEM[vaddr];
            }
        }
    }
}

void CPU_PIPE::print_history()
{
    json out_history;
    for (auto i = 0; i <= W.history_ID - 1; i++) {
        if (history_valid[i])
            out_history.push_back(history[i]);
    }

    std::cout << std::setw(4) << out_history << std::endl;
}

void CPU_PIPE::debug()
{
    for (int i = 0; i < 15; i++) {
        printf("%s: %lld\t", RG.get_reg_name(i), RG[i]);
        if (i % 4 == 3) {
            putchar('\n');
        }
    }
    putchar('\n');
    putchar('\n');

    printf("HID:\t%2d  Addr: 0x%x (%d)\n", Dnext.history_ID, Dnext.ins_addr, Dnext.ins_addr);
    printf("F:\tpredPC = %lld\tstall = %d\n", F.predPC, F.stall);
    printf("Fn:\tpredPC = %lld\tstall = %d\n", Fnext.predPC, Fnext.stall);
    printf("Dn:\tstat = %s\ticode = 0x%x\tifun = 0x%x\n", State_name[Dnext.stat], Dnext.icode, Dnext.ifun);
    printf("\trA = %s\trB = %s\tvalC = %lld\tvalP = %lld\n", RG.get_reg_name(Dnext.rA), RG.get_reg_name(Dnext.rB), Dnext.valC, Dnext.valP);
    printf("\tstall = %d\tbubble = %d\n", Dnext.stall, Dnext.bubble);
    putchar('\n');

    printf("HID:\t%2d  Addr: 0x%x (%d)\n", D.history_ID, D.ins_addr, D.ins_addr);
    printf("D:\tstat = %s\ticode = 0x%x\tifun = 0x%x\n", State_name[D.stat], D.icode, D.ifun);
    printf("\trA = %s\trB = %s\tvalC = %lld\tvalP = %lld\n", RG.get_reg_name(D.rA), RG.get_reg_name(D.rB), D.valC, D.valP);
    printf("\tstall = %d\tbubble = %d\n", D.stall, D.bubble);
    printf("En:\tstat = %s\ticode = 0x%x\tifun = 0x%x\n", State_name[Enext.stat], Enext.icode, Enext.ifun);
    printf("\tvalA = %lld\tvalB = %lld\tvalC = %lld\n", Enext.valA, Enext.valB, Enext.valC);
    printf("\tdstE = %s\tdstM = %s\tsrcA = %s\tsrcB = %s\n", RG.get_reg_name(Enext.dstE), RG.get_reg_name(Enext.dstM), RG.get_reg_name(Enext.srcA), RG.get_reg_name(Enext.srcB));
    printf("\tbubble = %d\n", Enext.bubble);
    putchar('\n');

    printf("HID:\t%2d  Addr: 0x%x (%d)\n", E.history_ID, E.ins_addr, E.ins_addr);
    printf("E:\tstat = %s\ticode = 0x%x\tifun = 0x%x\n", State_name[E.stat], E.icode, E.ifun);
    printf("\tvalA = %lld\tvalB = %lld\tvalC = %lld\n", E.valA, E.valB, E.valC);
    printf("\tdstE = %s\tdstM = %s\tsrcA = %s\tsrcB = %s\n", RG.get_reg_name(E.dstE), RG.get_reg_name(E.dstM), RG.get_reg_name(E.srcA), RG.get_reg_name(E.srcB));
    printf("\tbubble = %d\n", E.bubble);
    printf("Mn:\tstat = %s\ticode = 0x%x\tCnd = 0x%x\n", State_name[Mnext.stat], Mnext.icode, Mnext.Cnd);
    printf("\tvalA = %lld\tvalE = %lld\n", Mnext.valA, Mnext.valE);
    printf("\tdstE = %s\tdstM = %s\n", RG.get_reg_name(Mnext.dstE), RG.get_reg_name(Mnext.dstM));
    printf("\tbubble = %d\n", Mnext.bubble);
    putchar('\n');

    printf("HID:\t%2d  Addr: 0x%x (%d)\n", M.history_ID, M.ins_addr, M.ins_addr);
    printf("M:\tstat = %s\ticode = 0x%x\tCnd = 0x%x\n", State_name[M.stat], M.icode, M.Cnd);
    printf("\tvalA = %lld\tvalE = %lld\n", M.valA, M.valE);
    printf("\tdstE = %s\tdstM = %s\n", RG.get_reg_name(M.dstE), RG.get_reg_name(M.dstM));
    printf("\tbubble = %d\n", M.bubble);
    printf("Wn:\tstat = %s\ticode = 0x%x\n", State_name[Wnext.stat], Wnext.icode);
    printf("\tvalE = %lld\tvalM = %lld\n", Wnext.valE, Wnext.valM);
    printf("\tdstE = %s\tdstM = %s\n", RG.get_reg_name(Wnext.dstE), RG.get_reg_name(Wnext.dstM));
    printf("\tbubble = %d\n", Wnext.bubble);
    putchar('\n');

    printf("HID:\t%2d  Addr: 0x%x (%d)\n", W.history_ID, W.ins_addr, W.ins_addr);
    printf("W:\tstat = %s\ticode = 0x%x\n", State_name[W.stat], W.icode);
    printf("\tvalE = %lld\tvalM = %lld\n", W.valE, W.valM);
    printf("\tdstE = %s\tdstM = %s\n", RG.get_reg_name(W.dstE), RG.get_reg_name(W.dstM));
    printf("\tbubble = %d\n", W.bubble);
    putchar('\n');
    printf("---------- ---------- ----------\n\n");
}

bool CPU_PIPE::get_state(bool* cc, int* stat, _word_t* pc, _word_t* reg, int8_t* mem)
{
    int last_valid = Wnext.history_ID - 1;
    for (last_valid; last_valid >= 0 && history_valid[last_valid] == false; last_valid--)
        ;
    if (last_valid < 0) {
        // output an empty state
        cc[0] = 0;
        cc[1] = 0;
        cc[2] = 1;
        *stat = SAOK;
        *pc = 0;
        for (int i = 0; i < 15; i++) {
            reg[i] = 0;
        }
        for (_word_t vaddr = 0; vaddr < MSIZE; vaddr += 8) {
            *(_word_t*)(&mem[vaddr]) = DMEM[vaddr];
        }
        return true;
    }

    // debug
    // fprintf(stderr, "ID: %d\n", last_valid);
    // std::cerr << history[last_valid] << std::endl;

    cc[0] = (bool)(int)history[last_valid]["CC"]["OF"];
    cc[1] = (bool)(int)history[last_valid]["CC"]["SF"];
    cc[2] = (bool)(int)history[last_valid]["CC"]["ZF"];
    *stat = history[last_valid]["STAT"];
    *pc = history[last_valid]["PC"];

    for (int i = 0; i < 15; i++) {
        reg[i] = RG[i];
    }

    // MEM DUMP
    vector<_word_t> addr_record;
    for (auto& x : history[last_valid]["MEM"].items()) {
        _word_t vaddr;
        sscanf(x.key().c_str(), "%lld", &vaddr);
        ((_word_t*)mem)[vaddr] = x.value();
        addr_record.push_back(vaddr);
    }
    // handle 0 value
    for (_word_t vaddr = 0; vaddr < MSIZE; vaddr += 8) {
        if (std::find(addr_record.begin(), addr_record.end(), vaddr) == addr_record.end()) {
            *(_word_t*)(&mem[vaddr]) = 0;
        }
    }

    return true;
}

// ---------- exec ----------

void CPU_PIPE::exec(unsigned int n)
{
    // init pipeline
    if (history.size() == 0) {
        // insert bubble
        D.bubble = true;
        D.history_ID = -1;
        E.bubble = true;
        E.history_ID = -1;
        M.bubble = true;
        M.history_ID = -1;
        W.bubble = true;
        W.history_ID = -1;

        for (int i = 0; i < 4; i++)
            exec_once();
    }

    // execute
    for (unsigned int i = 0; i < n; i++) {
        if (!exec_once())
            break;
    }
}

bool CPU_PIPE::exec_once()
{
    if (Stat != SAOK) {
        return false;
    }

    fetch();
    decode();
    execute();
    memoryAccess();
    writeBack();

    // a part of Execute
    // these behavior require Wnext.stat (actually m_stat)
    // so must be done after state:memoryAccess
    if (Wnext.stat == SAOK) {
        CC = CCnext;
        Mnext.Cnd = calc_cnd(E.ifun);
    }
    if (E.icode == IRRMOVQ && !Mnext.Cnd) {
        Mnext.dstE = rnull;
    }

    // Forwarding
    if (D.icode != ICALL && D.icode != IJXX) {
        if (Mnext.dstE == Enext.srcA) {
            Enext.valA = Mnext.valE;    // valE from execute
            // puts("Forwarding valA from e_valE");
        } else if (M.dstM == Enext.srcA) {
            Enext.valA = Wnext.valM;    // valM from memory
            // puts("Forwarding valA from m_valm");
        } else if (M.dstE == Enext.srcA) {
            Enext.valA = M.valE;        // valE from memory
            // puts("Forwarding valA from M_valE");
        } else if (W.dstM == Enext.srcA) {
            Enext.valA = W.valM;        // valM from write back
            // puts("Forwarding valA from W_valM");
        } else if (W.dstE == Enext.srcA) {
            Enext.valA = W.valE;        // valE from write back
            // puts("Forwarding valA from W.valE");
        }
    }

    if (Mnext.dstE == Enext.srcB)
        Enext.valB = Mnext.valE;    // valE from execute
    else if (M.dstM == Enext.srcB)
        Enext.valB = Wnext.valM;    // valM from memory
    else if (M.dstE == Enext.srcB)
        Enext.valB = M.valE;        // valE from memory
    else if (W.dstM == Enext.srcB)
        Enext.valB = W.valM;        // valM from write back
    else if (W.dstE == Enext.srcB)
        Enext.valB = W.valE;        // valE from write back

    // handling special cases
    // 1. ret hazard
    if (D.icode == IRET || E.icode == IRET || M.icode == IRET) {
        // std::cout << "Ret Hazard!" << std::endl;
        Fnext.stall = true;
        Dnext.bubble = true;
    }
    // 2. load/use hazard
    if ((E.icode == IMRMOVQ || E.icode == IPOPQ) &&
        (E.dstM == Enext.srcA || E.dstM == Enext.srcB)) {
        // std::cout << "Load/Use Hazard!" << std::endl;
        Fnext.stall = true;
        Dnext.stall = true;
        Enext.bubble = true;
    }
    // 3. mispredicted branch
    if (Mnext.icode == IJXX && Mnext.ifun != 0 && (!!E.bp_taken ^ !!Mnext.Cnd)) {
        // std::cout << "Mis-branch Hazard!" << std::endl;
        Dnext.bubble = true;
        Enext.bubble = true;
    }

    // update history
    update_history();

    // debug
    // debug();

    // enter new cycle
    Stat = W.stat;
    if (!Fnext.stall) {
        F = Fnext;
    } else {
        F.predPC = Dnext.ins_addr;
        Fnext.stall = false;
    }
    if (!Dnext.stall) {
        D = Dnext;
    } else {
        // handle stall
        D.history_ID = Dnext.history_ID;
        Dnext.stall = false;
        Dnext.bubble = false;
    }
    E = Enext;
    M = Mnext;
    W = Wnext;

    return true;
}

bool CPU_PIPE::calc_cnd(int ifun)
{
    bool ret_val = (ifun == 0) ||                           // no condition
        (ifun == 0x1 && ((CC.SF ^ CC.OF) || (CC.ZF))) ||    // le
        (ifun == 0x2 && (CC.SF ^ CC.OF)) ||                 // l
        (ifun == 0x3 && CC.ZF) ||                           // e
        (ifun == 0x4 && !CC.ZF) ||                          // ne
        (ifun == 0x5 && !(CC.SF ^ CC.OF)) ||                // ge
        (ifun == 0x6 && (!(CC.SF ^ CC.OF) && !(CC.ZF)));    // g

    // fprintf(stderr, "%d(%d): ifun(%d), OF(%d), SF(%d), ZF(%d), RET(%d)\n", PC, history.size(), ifun, CC.OF, CC.SF, CC.ZF, ret_val);

    return ret_val;
}

bool addr_valid(_word_t vaddr)
{
    return vaddr <= MSIZE;
}

bool instr_valid(int icode)
{
    return 0 <= icode && icode <= IIADDQ;
}

bool need_regids(int icode)
{
    switch (icode) {
    case IRRMOVQ:
    case IIRMOVQ:
    case IRMMOVQ:
    case IMRMOVQ:
    case IOPQ:
    case IPUSHQ:
    case IPOPQ:
    case IIADDQ:
        return true;
    default:
        return false;
    }
}

bool need_valC(int icode)
{
    switch (icode) {
    case IIRMOVQ:
    case IRMMOVQ:
    case IMRMOVQ:
    case IJXX:
    case ICALL:
    case IIADDQ:
        return true;
    default:
        return false;
    }
}

void CPU_PIPE::fetch()
{
    _word_t f_pc;

    // select pc
    if (M.icode == IJXX && M.ifun != 0) {   // conditional jmp
        branch_update(M.ins_addr, M.Cnd);
        if ((!!M.bp_taken ^ !!M.Cnd)) { // mispredicted
            if (M.Cnd) {
                f_pc = M.valE;  // valC
            } else {
                f_pc = M.valA;  // valP
            }
        } else {                            // predict right
            f_pc = F.predPC;
        }
    } else if (W.icode == IRET) {       // ret
        f_pc = W.valM;
    } else {
        f_pc = F.predPC;
    }

    // create history record
    create_record(f_pc);

    Dnext.history_ID = history.size() - 1;
    Dnext.ins_addr = f_pc;
    Dnext.stat = SAOK;

    // fetch instruction
    if (!addr_valid(f_pc)) {
        Dnext.stat = SADR;
        return;
    }

    Instruction ins_ptr = DMEM.get_ins(f_pc);
    Dnext.icode = (*ins_ptr >> 4) & 0xF;
    Dnext.ifun = *ins_ptr & 0xF;
    ins_ptr++;

    if (!instr_valid(Dnext.icode)) {
        Dnext.stat = SINS;
        return;
    }

    if (Dnext.icode == IHALT) {
        Dnext.stat = SHLT;
        Fnext.predPC = f_pc;
        return;
    }

    if (need_regids(Dnext.icode)) {
        Dnext.rA = (*ins_ptr >> 4) & 0xF;
        Dnext.rB = *ins_ptr & 0xF;
        ins_ptr++;
    }

    if (need_valC(Dnext.icode)) {
        Dnext.valC = *(_word_t*)ins_ptr;
        ins_ptr += sizeof(_word_t);
    }

    Dnext.valP = DMEM.r2vaddr(ins_ptr);

    // predict PC
    if ((Dnext.icode == IJXX && Dnext.ifun == 0) || Dnext.icode == ICALL) { // normal jmp&call
        Fnext.predPC = Dnext.valC;
    } else if (Dnext.icode == IJXX && Dnext.ifun != 0) {    // conditional jmp
        if (branch_predict(f_pc)) {
            Fnext.predPC = Dnext.valC;
            Dnext.bp_taken = true;
        } else {
            Fnext.predPC = Dnext.valP;
            Dnext.bp_taken = false;
        }
    } else {
        Fnext.predPC = Dnext.valP;
    }
}

void CPU_PIPE::decode()
{
    Enext.history_ID = D.history_ID;
    Enext.ins_addr = D.ins_addr;
    Enext.bp_taken = D.bp_taken;

    if (D.bubble == true) {
        Enext.icode = INOP;
        Enext.stat = SAOK;
        Enext.bubble = true;
        Dnext.bubble = false;
        return;
    }

    Enext.stat = D.stat;
    Enext.icode = D.icode;
    Enext.ifun = D.ifun;
    Enext.valC = D.valC;

    int srcA;
    if (D.icode == IRRMOVQ ||
        D.icode == IRMMOVQ ||
        D.icode == IOPQ ||
        D.icode == IPUSHQ) {
        srcA = D.rA;
    } else if (D.icode == IPOPQ ||
        D.icode == IRET) {
        srcA = rsp;
    } else {
        srcA = rnull;
    }

    int srcB;
    if (D.icode == IRMMOVQ ||
        D.icode == IMRMOVQ ||
        D.icode == IOPQ ||
        D.icode == IIADDQ) {
        srcB = D.rB;
    } else if (D.icode == ICALL ||
        D.icode == IRET ||
        D.icode == IPUSHQ ||
        D.icode == IPOPQ) {
        srcB = rsp;
    } else {
        srcB = rnull;
    }

    if (D.icode == ICALL || D.icode == IJXX)
        Enext.valA = D.valP;
    else
        Enext.valA = RG[srcA];

    Enext.valB = RG[srcB];
    Enext.srcA = srcA;
    Enext.srcB = srcB;

    // printf("valA = %lld(maybe RG[%s]=%lld)\n", Enext.valA, RG.get_reg_name(srcA), RG[srcA]);

    int dstE;
    if (D.icode == IRRMOVQ ||
        D.icode == IIRMOVQ ||
        D.icode == IOPQ ||
        D.icode == IIADDQ) {
        dstE = D.rB;
    } else if (D.icode == IPUSHQ ||
        D.icode == IPOPQ ||
        D.icode == ICALL ||
        D.icode == IRET) {
        dstE = rsp;
    } else {
        dstE = rnull;
    }

    int dstM;
    if (D.icode == IMRMOVQ ||
        D.icode == IPOPQ) {
        dstM = D.rA;
    } else {
        dstM = rnull;
    }

    Enext.dstE = dstE;
    Enext.dstM = dstM;

}

void CPU_PIPE::execute()
{
    Mnext.history_ID = E.history_ID;
    Mnext.ins_addr = E.ins_addr;
    Mnext.bp_taken = E.bp_taken;

    if (E.bubble == true) {
        Mnext.icode = INOP;
        Mnext.stat = SAOK;
        Mnext.bubble = true;
        if (!D.bubble)
            Enext.bubble = false;
        return;
    }

    Mnext.stat = E.stat;
    Mnext.icode = E.icode;
    Mnext.ifun = E.ifun;
    Mnext.valA = E.valA;

    _word_t aluA = 0;
    _word_t aluB = 0;

    if (E.icode == IRRMOVQ || E.icode == IOPQ) {
        aluA = E.valA;
    } else if (E.icode == IIRMOVQ || E.icode == IRMMOVQ || E.icode == IMRMOVQ || E.icode == IIADDQ || E.icode == IJXX) {
        aluA = E.valC;
    } else if (E.icode == ICALL || E.icode == IPUSHQ) {
        aluA = -8;
    } else if (E.icode == IRET || E.icode == IPOPQ) {
        aluA = 8;
    }

    if (E.icode == IRMMOVQ ||
        E.icode == IMRMOVQ ||
        E.icode == IOPQ ||
        E.icode == ICALL ||
        E.icode == IPUSHQ ||
        E.icode == IRET ||
        E.icode == IPOPQ ||
        E.icode == IIADDQ) {
        aluB = E.valB;
    } else if (E.icode == IRRMOVQ || E.icode == IIRMOVQ || E.icode == IJXX) {
        aluB = 0;
    }

    int alufun = ALUADD;
    if (E.icode == IOPQ) {
        alufun = E.ifun;
    }

    // ALU
    _sword_t valE;
    if (alufun == ALUADD) {
        valE = aluA + aluB;
    } else if (alufun == ALUSUB) {
        aluA = -aluA;
        valE = aluA + aluB;
    } else if (alufun == ALUAND) {
        valE = aluA & aluB;
    } else if (alufun == ALUXOR) {
        valE = aluA ^ aluB;
    }
    Mnext.valE = valE;

    // set CC & calc CND
    if (W.stat == SAOK && (E.icode == IOPQ || E.icode == IIADDQ)) {
        if (alufun == ALUADD || alufun == ALUSUB) {
            if (aluA > 0 && aluB > 0 && Mnext.valE < 0 ||
                aluA < 0 && aluB < 0 && Mnext.valE > 0) {
                CCnext.OF = 1;
            } else {
                CCnext.OF = 0;
            }
        } else {
            CCnext.OF = 0;
        }
        CCnext.SF = valE < 0;
        CCnext.ZF = valE == 0;
    } else {
        CCnext = CC;
    }

    Mnext.dstE = E.dstE;
    Mnext.dstM = E.dstM;

    // these will be done in exec_once
    // Mnext.Cnd = calc_cnd(E.ifun);
    // if (E.icode == IRRMOVQ && !Mnext.Cnd)
    //     Mnext.dstE = rnull;
}

void CPU_PIPE::memoryAccess()
{
    Wnext.history_ID = M.history_ID;
    Wnext.ins_addr = M.ins_addr;

    if (M.bubble == true) {
        Wnext.icode = INOP;
        Wnext.stat = SAOK;
        Wnext.bubble = true;
        if (!E.bubble)
            Mnext.bubble = false;
        return;
    }

    _word_t mem_addr;
    if (M.icode == IRMMOVQ || M.icode == IPUSHQ || M.icode == ICALL || M.icode == IMRMOVQ) {
        mem_addr = M.valE;
    } else if (M.icode == IPOPQ || M.icode == IRET) {
        mem_addr = M.valA;
    } else {
        mem_addr = 0;   // a random valid address
    }

    Wnext.stat = M.stat;
    if (!addr_valid(mem_addr)) {
        Wnext.stat = SADR;
    }

    bool mem_read = false;
    bool mem_write = false;
    if (M.icode == IMRMOVQ || M.icode == IPOPQ || M.icode == IRET) {
        mem_read = true;
    } else if (M.icode == IRMMOVQ || M.icode == IPUSHQ || M.icode == ICALL) {
        mem_write = true;
    }

    if (mem_write) {
        DMEM[mem_addr] = M.valA;
    }

    Wnext.icode = M.icode;
    Wnext.valE = M.valE;
    Wnext.valM = DMEM[mem_addr];
    Wnext.dstE = M.dstE;
    Wnext.dstM = M.dstM;
}

void CPU_PIPE::writeBack()
{
    if (W.bubble) {
        if (!M.bubble)
            Wnext.bubble = false;
        if (W.history_ID >= 0) {    // 非初始bubble
            // 将本bubble对应的记录设置为非法
            history_valid[W.history_ID] = false;
        }
        return;
    } else {
        auto last_valid = W.history_ID - 1;
        if (last_valid >= 0) {
            for (last_valid; history_valid[last_valid] == false; last_valid--)
                ;
            if (last_valid >= 0)
                history[last_valid]["PC"] = W.ins_addr;
        }
    }

    RG[W.dstE] = W.valE;
    RG[W.dstM] = W.valM;
}

// ---------- branch predict ----------

bool CPU_PIPE::branch_predict(_word_t pc)
{
    switch (strategy) {
    case Never_Taken:
        return false;
    case Always_Taken:
        return true;
    case Branch_Predict_Bimodal:
        PredictorState state = bp_buffer[pc % PRED_BUF_SIZE];
        if (state == STRONG_TAKEN || state == WEAK_TAKEN) {
            return true;
        } else if (state == STRONG_NOT_TAKEN || state == WEAK_NOT_TAKEN) {
            return false;
        }
    }
}

void CPU_PIPE::branch_update(_word_t pc, bool branch)
{
    if (strategy != Branch_Predict_Bimodal) return;

    int index = pc % PRED_BUF_SIZE;
    PredictorState state = this->bp_buffer[index];
    if (branch) {
        if (state == STRONG_NOT_TAKEN) {
            this->bp_buffer[index] = WEAK_NOT_TAKEN;
        } else if (state == WEAK_NOT_TAKEN) {
            this->bp_buffer[index] = WEAK_TAKEN;
        } else if (state == WEAK_TAKEN) {
            this->bp_buffer[index] = STRONG_TAKEN;
        }
        // do nothing if STRONG_TAKEN
    } else {        // not taken
        if (state == STRONG_TAKEN) {
            this->bp_buffer[index] = WEAK_TAKEN;
        } else if (state == WEAK_TAKEN) {
            this->bp_buffer[index] = WEAK_NOT_TAKEN;
        } else if (state == WEAK_NOT_TAKEN) {
            this->bp_buffer[index] = STRONG_NOT_TAKEN;
        }
        // do noting if STRONG_NOT_TAKEN
    }
}
