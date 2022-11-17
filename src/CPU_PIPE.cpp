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
    Stat = AOK;
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
    if (Stat != AOK) {
        // fprintf(stderr, "EXEC FAIL (Stat: %s)\n", State_name[Stat]);
        return;
    }
    for (unsigned int i = 0; i < n; i++) {
        if (!addr_check(PC)) {
            break;
        }

        PC += exec_once(DMEM.get_ins(PC));
        update_history();

        if (Stat != AOK) {
            break;
        }
    }
    if (Stat != AOK) {
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

}

bool CPU::calc_cnd(int ifun)
{
    bool ret_val = (ifun == 0) ||                                    // no condition
        (ifun == 0x1 && ((CC.SF ^ CC.OF) || (CC.ZF))) ||  // le
        (ifun == 0x2 && (CC.SF ^ CC.OF)) ||              // l
        (ifun == 0x3 && CC.ZF) ||                        // e
        (ifun == 0x4 && !CC.ZF) ||                       // ne
        (ifun == 0x5 && !(CC.SF ^ CC.OF)) ||             // ge
        (ifun == 0x6 && (!(CC.SF ^ CC.OF) && !(CC.ZF)));   // g
        
    // fprintf(stderr, "%d(%d): ifun(%d), OF(%d), SF(%d), ZF(%d), RET(%d)\n", PC, history.size(), ifun, CC.OF, CC.SF, CC.ZF, ret_val);

    return ret_val;
}

bool CPU::addr_check(_word_t vaddr)
{
    if (vaddr > MSIZE) {
        Stat = ADR;
        return false;
    } else {
        return true;
    }
}