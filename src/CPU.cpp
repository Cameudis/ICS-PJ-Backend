#include "CPU.h"
#include <cstring>

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
    
}

void CPU::load_prog(std::ifstream& infile)
{
    char str[100];
    word_t vaddr = 0;

    char byte_s[3] = "00";
    uint8_t byte;

    while (!infile.eof()) {
        // read instruction
        infile.getline(str, 100, '|');

        char* ins_str = strchr(str, ':');
        if (ins_str) {
            ins_str += 2;   // jmp ": "

            while (*ins_str && !isspace(*ins_str)) {
                strncpy(byte_s, ins_str, 2);
                sscanf(byte_s, "%x", &byte);
                *DMEM.v2raddr(vaddr++) = byte;

                ins_str += 2;
            }
        }

        // jmp asm/comments
        infile.getline(str, 100, '\n');
    }
}

// ---------- exec ----------

void CPU::exec(int n)
{
    if (Stat != AOK) {
        printf("EXEC FAIL (Stat: %s)\n", State_name[Stat]);
        return;
    }
    for (int i = 0; i < n; i++) {
        PC += exec_once(DMEM.get_ins(PC));

        if (Stat != AOK)
            break;
    }
    if (Stat != AOK) {
        printf("EXEC HALT (Stat: %s)\n", State_name[Stat]);
        return;
    }
}

int CPU::exec_once(Instruction ins)
{
    int ifunc = ins[0] & 0xF;

    return (this->*(instab[ifunc]))(ins);
}

int CPU::ins_halt(Instruction ins)
{
    Stat = HLT;
    return 1;
}

int CPU::ins_nop(Instruction ins)
{
//                              _ooOoo_
//                             o8888888o
//                             88" . "88
//                             (| -_- |)
//                              O\ = /O
//                           ____/`---'\____
//                        .   ' \\| |// `.
//                         / \\||| : |||// \
//                        / _||||| -:- |||||- \
//                         | | \\\ - /// | |
//                       | \_| ''\---/'' | |
//                        \ .-\__ `-` ___/-. /
//                    ___`. .' /--.--\ `. . __
//                  ."" '< `.___\_<|>_/___.' >'"".
//                 | | : `- \`.;`\ _ /`;.`/ - ` : | |
//                    \ \ `-. \_ __\ /__ _/ .-` / /
//           ======`-.____`-.___\_____/___.-`____.-'======
//                              `=---='
//
//           .............................................
//                     佛祖保佑             永无BUG
    return 1;
}

int CPU::ins_rrmov(Instruction ins)
{
    int icode = (ins[0]>>4) & 0xF;
    if (ccjudge(icode)) {
        int ra = ins[1] & 0x0F;
        int rb = ins[1] & 0xF0;
        RG[rb] = RG[ra];
    }
    return 2;
}

int CPU::ins_irmov(Instruction ins)
{
    int rb = (ins[1]>>4) & 0xF;
    RG[rb] = *(word_t*)(&ins[2]);

    return 2 + sizeof(word_t);
}

int CPU::ins_rmmov(Instruction ins)
{
    int ra = ins[1] & 0xF;
    int rb = (ins[1]>>4) & 0xF;
    word_t offset = *(word_t*)(&ins[2]);
    
    DMEM[offset + RG[rb]] = RG[ra];

    return 2 + sizeof(word_t);
}

int CPU::ins_mrmov(Instruction ins)
{
    int ra = ins[1] & 0xF;
    int rb = (ins[1]>>4) & 0xF;
    word_t offset = *(word_t*)(&ins[2]);
    
    RG[ra] = DMEM[offset + RG[rb]];

    return 2 + sizeof(word_t);
}

int CPU::ins_op(Instruction ins)
{
    int icode = (ins[0]>>4) & 0xF;
    int ra = ins[1] & 0xF;
    int rb = (ins[1]>>4) & 0xF;

    switch (icode) {
        case 0x0:
            RG[rb] += RG[ra];
            break;
        case 0x1:
            RG[rb] -= RG[ra];
            break;
        case 0x2:
            RG[rb] &= RG[ra];
            break;
        case 0x3:
            RG[rb] ^= RG[ra];
            break;

        default:
            assert(0);
    }

    return 2;
}

int CPU::ins_jmp(Instruction ins)
{
    int icode = (ins[0]>>4) & 0xF;

    if (ccjudge(icode)) {
        PC = *(word_t*)(&ins[1]);
        return 0;
    } else {
        return 1 + sizeof(word_t);
    }
}

int CPU::ins_call(Instruction ins)
{
    RG[4] -= sizeof(word_t);                // update & push rip
    DMEM[RG[4]] = PC + 1 + sizeof(word_t);
    PC = *(word_t*)(&ins[1]);               // jmp addr

    return 0;
}

int CPU::ins_ret(Instruction ins)
{
    PC = DMEM[RG[4]];                       // pop rip
    RG[4] += sizeof(word_t);

    return 0;
}

int CPU::ins_push(Instruction ins)
{
    int ra = ins[1] & 0xF;

    DMEM[RG[4] - sizeof(word_t)] = RG[ra];
    RG[4] -= sizeof(word_t);

    return 2;
}

int CPU::ins_pop(Instruction ins)
{
    int ra = ins[1] & 0xF;

    RG[ra] = DMEM[RG[4]];
    RG[4] += sizeof(word_t);

    return 2;
}

int CPU::ins_null_handler(Instruction ins)
{
    Stat = INS;
    return 0;
}

bool CPU::ccjudge(int icode)
{
    return !icode ||                                    // no condition
        icode == 0x1 && ((CC.SF ^ CC.OF) | (CC.ZF)) ||  // le
        icode == 0x2 && (CC.SF ^ CC.OF) ||              // l
        icode == 0x3 && CC.ZF ||                        // e
        icode == 0x4 && !CC.ZF ||                       // ne
        icode == 0x5 && ~((CC.SF ^ CC.OF) | (CC.ZF)) || // ge
        icode == 0x6 && ~(CC.SF ^ CC.OF);               // g
}