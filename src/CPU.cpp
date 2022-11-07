#include "CPU.h"
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
    Stat = AOK;
    PC = 0;
}

void CPU::load_prog(std::istream& infile)
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

    update_history();
}

// ---------- output ----------

void CPU::update_history()
{
    json crt;

    // PC dump
    crt["PC"] = PC;

    // REG dump
    for (int i = 0; i < 15; i++) {
        crt["REG"][RG.get_reg_name(i)] = RG[i];
    }

    // CC dump
    crt["CC"]["ZF"] = CC.ZF;
    crt["CC"]["SF"] = CC.SF;
    crt["CC"]["OF"] = CC.OF;

    // STAT dump
    crt["STAT"] = Stat;

    // MEM dump
    // waiting for Optimization!!!
    for (word_t vaddr = 0; vaddr < MSIZE; vaddr += 8) {
        if (DMEM[vaddr]) {
            char vaddr_str[10];
            sprintf(vaddr_str, "%d", vaddr);

            crt["MEM"][vaddr_str] = DMEM[vaddr];
        }
    }

    // add to history
    history.push_back(crt);
}

// ---------- exec ----------

void CPU::exec(unsigned int n)
{
    if (Stat != AOK) {
        fprintf(stderr, "EXEC FAIL (Stat: %s)\n", State_name[Stat]);
        return;
    }
    for (int i = 0; i < n; i++) {
        PC += exec_once(DMEM.get_ins(PC));
        update_history();

        if (Stat != AOK) {
            break;
        }
    }
    if (Stat != AOK) {
        fprintf(stderr, "EXEC HALT (Stat: %s)\n", State_name[Stat]);
        return;
    }
}

void CPU::back(unsigned int n)
{
    unsigned int des_id = history.size() - 1 - n;
    if ((int)des_id < 0) {
        fprintf(stderr, "ERROR: back too much");
    }

    // recover state
    memset(&DMEM[0], 0, MSIZE);

    PC = history[des_id]["PC"];

    for (int i = 0; i < 15; i++) {
        RG[i] = history[des_id]["REG"][RG.get_reg_name(i)];
    }

    CC.ZF = history[des_id]["CC"]["ZF"];
    CC.SF = history[des_id]["CC"]["SF"];
    CC.OF = history[des_id]["CC"]["OF"];

    Stat = history[des_id]["STAT"];

    map<string, word_t> mem2val = history[des_id]["MEM"].get<map<string, word_t>>();
    for (auto& x: mem2val) {
        word_t addr;
        sscanf(x.first.c_str(), "%ld", &addr);
        DMEM[addr] = x.second;
    }

    // 上白沢慧音 転世「一条戻り橋」
    history.erase(history.begin()+ des_id + 1, history.end());
}

void CPU::im_exec(Instruction ins)
{
    exec_once(ins);
}

int CPU::exec_once(Instruction ins)
{
    int icode = (ins[0]>>4) & 0xF;

    return (this->*(instab[icode]))(ins);
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
    int ifunc = ins[0] & 0xF;
    if (ccjudge(ifunc)) {
        int ra = (ins[1]>>4) & 0xF;
        int rb = ins[1] & 0xF;
        RG[rb] = RG[ra];
    }
    return 2;
}

int CPU::ins_irmov(Instruction ins)
{
    int rb = ins[1] & 0xF;
    RG[rb] = *(word_t*)(&ins[2]);

    return 2 + sizeof(word_t);
}

int CPU::ins_rmmov(Instruction ins)
{
    int ra = (ins[1]>>4) & 0xF;
    int rb = ins[1] & 0xF;
    word_t offset = *(word_t*)(&ins[2]);
    
    DMEM[offset + RG[rb]] = RG[ra];

    return 2 + sizeof(word_t);
}

int CPU::ins_mrmov(Instruction ins)
{
    int ra = (ins[1]>>4) & 0xF;
    int rb = ins[1] & 0xF;
    word_t offset = *(word_t*)(&ins[2]);
    
    RG[ra] = DMEM[offset + RG[rb]];

    return 2 + sizeof(word_t);
}

int CPU::ins_op(Instruction ins)
{
    int ifunc = ins[0] & 0xF;
    int ra = (ins[1]>>4) & 0xF;
    int rb = ins[1] & 0xF;

    sword_t b = RG[rb];         // temp save for condition code setting
    switch (ifunc) {
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

    // set CC
    sword_t a = RG[ra];
    sword_t result = RG[rb];
    if (a >= 0 && b >= 0 && result < a ||
        a <= 0 && b <= 0 && result > a) {
        CC.OF = 1;
    } else {
        CC.OF = 0;
    }
    CC.SF = result < 0;
    CC.ZF = result == 0;

    return 2;
}

int CPU::ins_jmp(Instruction ins)
{
    int ifunc = ins[0] & 0xF;

    if (ccjudge(ifunc)) {
        PC = *(word_t*)(&ins[1]);
        return 0;
    } else {
        return 1 + sizeof(word_t);
    }
}

int CPU::ins_call(Instruction ins)
{
    RG[rsp] -= sizeof(word_t);                // update & push rip
    DMEM[RG[rsp]] = PC + 1 + sizeof(word_t);
    PC = *(word_t*)(&ins[1]);               // jmp addr

    return 0;
}

int CPU::ins_ret(Instruction ins)
{
    PC = DMEM[RG[rsp]];                       // pop rip
    RG[rsp] += sizeof(word_t);

    return 0;
}

int CPU::ins_push(Instruction ins)
{
    int ra = (ins[1]>>4) & 0xF;

    DMEM[RG[rsp] - sizeof(word_t)] = RG[ra];
    RG[rsp] -= sizeof(word_t);

    return 2;
}

int CPU::ins_pop(Instruction ins)
{
    int ra = (ins[1]>>4) & 0xF;

    RG[ra] = DMEM[RG[rsp]];
    RG[rsp] += sizeof(word_t);

    return 2;
}

int CPU::ins_null_handler(Instruction ins)
{
    Stat = INS;
    return 0;
}

bool CPU::ccjudge(int ifunc)
{
    return !ifunc ||                                    // no condition
        ifunc == 0x1 && ((CC.SF ^ CC.OF) | (CC.ZF)) ||  // le
        ifunc == 0x2 && (CC.SF ^ CC.OF) ||              // l
        ifunc == 0x3 && CC.ZF ||                        // e
        ifunc == 0x4 && !CC.ZF ||                       // ne
        ifunc == 0x5 && ~((CC.SF ^ CC.OF) | (CC.ZF)) || // ge
        ifunc == 0x6 && ~(CC.SF ^ CC.OF);               // g
}