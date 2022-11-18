#include "CPU_PIPE.h"
#include <cstring>
#include <map>
#include <string>
using std::map; using std::string;

static const char* State_name[] = {
    "NULL",     // 0
    "AOK",      // 1
    "HLT",      // 2
    "ADR",      // 3
    "INS"       // 4
};

// ---------- init ----------

CPU::CPU()
{
    reset();
}

void CPU::reset()
{
    PC = 0;
    Stat = SAOK;
    CC.ZF = 1;
    CC.OF = 0;
    CC.SF = 0;
    DMEM.clear();
    RG.clear();
}

void CPU::load_prog(std::istream& infile)
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

    // update_history();
    // fprintf(stderr, "Done!\n");
}

// ---------- output ----------

void CPU::update_history()
{
    json crt;

    // PC dump
    crt["PC"] = PC;

    // REG dump
    for (int i = 0; i < 15; i++) {
        crt["REG"][RG.get_reg_name(i)] = (int64_t)RG[i];
    }

    // CC dump
    crt["CC"]["OF"] = (int)CC.OF;
    crt["CC"]["SF"] = (int)CC.SF;
    crt["CC"]["ZF"] = (int)CC.ZF;

    // STAT dump
    crt["STAT"] = Stat;

    // MEM dump
    // waiting for Optimization!!!
    for (_word_t vaddr = 0; vaddr < MSIZE; vaddr += 8) {
        if (DMEM[vaddr]) {
            char vaddr_str[10];
            sprintf(vaddr_str, "%lld", vaddr);

            crt["MEM"][vaddr_str] = (int64_t)DMEM[vaddr];
        }
    }

    // add to history
    history.push_back(crt);
}

// ---------- exec ----------

void CPU::exec(unsigned int n)
{
    if (Stat != SAOK) {
        // fprintf(stderr, "EXEC FAIL (Stat: %s)\n", State_name[Stat]);
        return;
    }
    for (unsigned int i = 0; i < n; i++) {
        if (!addr_check(PC)) {
            break;
        }

        PC += exec_once(DMEM.get_ins(PC));
        update_history();

        if (Stat != SAOK) {
            break;
        }
    }
    if (Stat != SAOK) {
        // fprintf(stderr, "EXEC HALT (Stat: %s)\n", State_name[Stat]);
        return;
    }
}

bool CPU::back(unsigned int n)
{
    /* TODO */
}

void CPU::im_exec(Instruction ins)
{
    /* TODO */
}

int CPU::exec_once(Instruction ins)
{
    fetch();
    decode();
    execute();
    memoryAccess();
    writeBack();

    // special case
    // these behavior require Wnext, so must be done after state:memoryAccess
    if (Wnext.stat == SAOK) {
        CC = CCnext;
        Mnext.Cnd = calc_cnd(E.ifun);
    }
    if (E.icode == IRRMOVQ && !Mnext.Cnd) {
        Mnext.dstE = rnull;
    }
    if (Mnext.dstE == Enext.srcA) {     // forwarding
        Enext.valA = Mnext.valE;
    }

}

bool CPU::calc_cnd(int ifun)
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

bool CPU::addr_check(_word_t vaddr)
{
    if (vaddr > MSIZE) {
        Stat = SADR;
        return false;
    } else {
        return true;
    }
}

bool instr_valid(int icode)
{
    return 0 <= icode && icode <= 0xB;
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
            return true;
        default:
            return false;
    }
}

bool need_valC(int icode) {
    switch (icode) {
        case IIRMOVQ:
        case IRMMOVQ:
        case IMRMOVQ:
        case IJXX:
        case ICALL:
            return true;
        default:
            return false;
    }
}

void CPU::fetch()
{
    _word_t f_pc;

    // select pc
    if ((M.icode == IJXX) && M.Cnd) {   // conditional jmp
        f_pc = M.valA;
    } else if (W.icode == IRET) {       // ret
        f_pc = W.valM;
    } else {
        f_pc = F.predPC;
    }

    // fetch instruction
    if (!addr_check(f_pc)) {
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
    if (Dnext.icode == IJXX || Dnext.icode == ICALL) {
        Fnext.predPC = Dnext.valC;
    } else {
        Fnext.predPC = Dnext.valP;
    }
}

void CPU::decode()
{
    Enext.stat = D.stat;
    Enext.icode = D.icode;
    Enext.ifun = D.ifun;
    Enext.valC = D.valC;

    int srcA;
    if (D.icode == IRRMOVQ || 
        D.icode == IRMMOVQ || 
        D.icode == IOPQ || 
        D.icode == IPUSHQ){
            srcA = D.rA;
    }
    else if (D.icode == IPOPQ ||
             D.icode == IRET) {
                srcA = rsp;
    }
    else {
        srcA = rnull;
    }

    int srcB;
    if (D.icode == IRMMOVQ ||
        D.icode == IMRMOVQ ||
        D.icode == IOPQ) {
            srcB = D.rB;
    }
    else if (D.icode == ICALL ||
             D.icode == IRET ||
             D.icode == IPUSHQ ||
             D.icode == IPOPQ) {
                srcB = rsp;
    }
    else {
        srcB = rnull;
    }

    if (D.icode == ICALL || D.icode == IJXX)
        Enext.valA = D.valP;
    else
        Enext.valA = RG[srcA];

    Enext.valB = RG[srcB];
    Enext.srcA = srcA;
    Enext.srcB = srcB;

    int dstE;
    if (D.icode == IRRMOVQ ||
        D.icode == IIRMOVQ ||
        D.icode == IOPQ) {
            dstE = D.rB;
    }
    else if (D.icode == IPUSHQ ||
             D.icode == IPOPQ ||
             D.icode == ICALL ||
             D.icode == IRET) {
                dstE = rsp;
    }
    else {
        dstE = rnull;
    }

    int dstM;
    if (D.icode == IMRMOVQ ||
        D.icode == IPOPQ) {
            dstM = D.rA;
    }
    else {
        dstM = rnull;
    }

    Enext.dstE = dstE;
    Enext.dstM = dstM;

}

void CPU::execute()
{
    Mnext.stat = E.stat;
    Mnext.icode = E.icode;
    Mnext.valA = E.valA;

    _word_t aluA = 0;
    _word_t aluB = 0;

    if (E.icode == IRRMOVQ || E.icode == IOPQ) {
        aluA = E.valA;
    } else if (E.icode == IIRMOVQ || E.icode == IRMMOVQ || E.icode == IMRMOVQ) {
        aluA = E.valC;
    } else if (E.icode == ICALL || E.icode == IPUSHQ) {
        aluA = -8;
    } else if (E.icode == IRET || E.icode == IOPQ) {
        aluA = 8;
    }

    if (E.icode == IRMMOVQ ||
        E.icode == IMRMOVQ ||
        E.icode == IOPQ ||
        E.icode == ICALL ||
        E.icode == IPUSHQ ||
        E.icode == IRET ||
        E.icode == IPOPQ) {
        aluB = E.valB;
    } else if (E.icode == IRRMOVQ || E.icode == IIRMOVQ) {
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

    } else if (alufun == ALUXOR) {

    }

    // set CC & calc CND
    // 111111111111
    if (W.stat == SAOK) {
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

    // these will be done in exec_once
    // Mnext.Cnd = calc_cnd(E.ifun);
    // if (E.icode == IRRMOVQ && !Mnext.Cnd)
    //     Mnext.dstE = rnull;

    Mnext.dstM = E.dstM;
}

void CPU::memoryAccess()
{
    _word_t mem_addr;
    if (M.icode == IRRMOVQ || M.icode == IPUSHQ || M.icode == ICALL || M.icode == IMRMOVQ) {
        mem_addr = M.valE;
    } else if (M.icode == IPOPQ || M.icode == IRET) {
        mem_addr = M.valA;
    }

    if (!addr_check(mem_addr)) {
        Wnext.stat = SADR;
        return;
    }

    
}

void CPU::writeBack()
{

}
