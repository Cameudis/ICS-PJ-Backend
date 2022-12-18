#include "CPU_SEQ.h"
#include <cstring>
#include <iostream>
#include <map>
#include <string>
using std::map; using std::string;

// ---------- init ----------

CPU_SEQ::CPU_SEQ()
{
    reset();
}

void CPU_SEQ::reset()
{
    PC = 0;
    Stat = SAOK;
    CC.ZF = 1;
    CC.OF = 0;
    CC.SF = 0;
    DMEM.clear();
    RG.clear();
    history.clear();
}

void CPU_SEQ::load_prog(std::istream& infile)
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

    update_history();
    // fprintf(stderr, "Done!\n");
}

// ---------- output ----------

void CPU_SEQ::update_history()
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

void CPU_SEQ::print_history()
{
    json output;
    for (int i = 1; i < history.size(); i++) {
        output.push_back(history[i]);
    }
    std::cout << std::setw(4) << output << std::endl;
}

bool CPU_SEQ::get_state(bool* cc, int* stat, _word_t* pc, _word_t* reg, int8_t* mem)
{
    cc[0] = CC.OF;
    cc[1] = CC.SF;
    cc[2] = CC.ZF;

    *stat = Stat;
    *pc = PC;

    for (int i = 0; i < 15; i++) {
        reg[i] = RG[i];
    }

    memcpy(mem, DMEM.v2raddr(0), MSIZE);

    return true;
}

// ---------- exec ----------

void CPU_SEQ::exec(unsigned int n)
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

bool CPU_SEQ::back(unsigned int n)
{
    unsigned int des_id = history.size() - 1 - n;
    if ((int)des_id < 0) {
        fprintf(stderr, "ERROR: back too much");
        return false;
    }

    // recover state
    memset(&DMEM[0], 0, MSIZE);

    PC = history[des_id]["PC"];

    for (int i = 0; i < 15; i++) {
        RG[i] = history[des_id]["REG"][RG.get_reg_name(i)];
    }

    CC.ZF = (bool)(int)history[des_id]["CC"]["ZF"];
    CC.SF = (bool)(int)history[des_id]["CC"]["SF"];
    CC.OF = (bool)(int)history[des_id]["CC"]["OF"];

    Stat = history[des_id]["STAT"];

    map<string, _word_t> mem2val = history[des_id]["MEM"].get<map<string, _word_t>>();
    for (auto& x : mem2val) {
        _word_t vaddr;
        sscanf(x.first.c_str(), "%lld", &vaddr);
        DMEM[vaddr] = x.second;
    }

    // 上白沢慧音 転世「一条戻り橋」
    history.erase(history.begin() + des_id + 1, history.end());
    return true;
}

void CPU_SEQ::im_exec(Instruction ins)
{
    exec_once(ins);
    update_history();
}

int CPU_SEQ::exec_once(Instruction ins)
{
    int icode = (ins[0] >> 4) & 0xF;

    return (this->*(instab[icode]))(ins);
}

int CPU_SEQ::ins_halt(Instruction ins)
{
    Stat = SHLT;
    return 0;
}

int CPU_SEQ::ins_nop(Instruction ins)
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

int CPU_SEQ::ins_rrmov(Instruction ins)
{
    int ifun = ins[0] & 0xF;
    if (calc_cnd(ifun)) {
        int ra = (ins[1] >> 4) & 0xF;
        int rb = ins[1] & 0xF;
        RG[rb] = RG[ra];
    }
    return 2;
}

int CPU_SEQ::ins_irmov(Instruction ins)
{
    int rb = ins[1] & 0xF;
    RG[rb] = *(_word_t*)(&ins[2]);

    return 2 + sizeof(_word_t);
}

int CPU_SEQ::ins_rmmov(Instruction ins)
{
    int ra = (ins[1] >> 4) & 0xF;
    int rb = ins[1] & 0xF;
    _word_t offset = *(_word_t*)(&ins[2]);

    if (!addr_check(offset + RG[rb]))   return 0;

    DMEM[offset + RG[rb]] = RG[ra];

    return 2 + sizeof(_word_t);
}

int CPU_SEQ::ins_mrmov(Instruction ins)
{
    int ra = (ins[1] >> 4) & 0xF;
    int rb = ins[1] & 0xF;
    _word_t offset = *(_word_t*)(&ins[2]);

    if (!addr_check(offset + RG[rb]))   return 0;

    RG[ra] = DMEM[offset + RG[rb]];

    return 2 + sizeof(_word_t);
}

int CPU_SEQ::ins_op(Instruction ins)
{
    int ifun = ins[0] & 0xF;
    int ra = (ins[1] >> 4) & 0xF;
    int rb = ins[1] & 0xF;

    _sword_t a = RG[ra];
    _sword_t b = RG[rb];
    _sword_t result;
    switch (ifun) {
    case 0x0:
        result = b + a;
        if (a > 0 && b > 0 && result < 0 ||
            a < 0 && b < 0 && result > 0) {
            CC.OF = 1;
        } else {
            CC.OF = 0;
        }

        // fprintf(stderr, "%d(%d): %lld + %lld = %lld\n", PC, history.size(), b, a, RG[rb]);
        break;
    case 0x1:
        result = b - a;
        if (a < 0 && b > 0 && result < 0 ||
            a > 0 && b < 0 && result > 0) {
            CC.OF = 1;
        } else {
            CC.OF = 0;
        }
        // fprintf(stderr, "%d(%d): %lld - %lld = %lld\n", PC, history.size(), b, a, RG[rb]);
        break;
    case 0x2:
        result = a & b;
        CC.OF = 0;
        // fprintf(stderr, "%d(%d): %lld & %lld = %lld\n", PC, history.size(), b, a, RG[rb]);
        break;
    case 0x3:
        result = a ^ b;
        CC.OF = 0;
        // fprintf(stderr, "%d(%d): %lld ^ %lld = %lld\n", PC, history.size(), b, a, RG[rb]);
        break;

    default:
        assert(0);
    }
    RG[rb] = result;

    // set CC
    CC.SF = result < 0;
    CC.ZF = result == 0;

    return 2;
}

int CPU_SEQ::ins_jmp(Instruction ins)
{
    int ifun = ins[0] & 0xF;

    if (calc_cnd(ifun)) {
        PC = *(_word_t*)(&ins[1]);
        return 0;
    } else {
        return 1 + sizeof(_word_t);
    }
}

int CPU_SEQ::ins_call(Instruction ins)
{
    RG[rsp] -= sizeof(_word_t);                // update & push rip
    DMEM[RG[rsp]] = PC + 1 + sizeof(_word_t);
    PC = *(_word_t*)(&ins[1]);               // jmp addr

    return 0;
}

int CPU_SEQ::ins_ret(Instruction ins)
{
    if (!addr_check(RG[rsp]))   return 0;

    PC = DMEM[RG[rsp]];                       // pop rip
    RG[rsp] += sizeof(_word_t);

    return 0;
}

int CPU_SEQ::ins_push(Instruction ins)
{
    int ra = (ins[1] >> 4) & 0xF;

    if (!addr_check(RG[rsp] - sizeof(_word_t))) {
        RG[rsp] -= sizeof(_word_t);
        return 0;
    } else {
        DMEM[RG[rsp] - sizeof(_word_t)] = RG[ra];
        RG[rsp] -= sizeof(_word_t);
        return 2;
    }
}

int CPU_SEQ::ins_pop(Instruction ins)
{
    if (!addr_check(RG[rsp]))   return 0;

    int ra = (ins[1] >> 4) & 0xF;

    RG[rsp] += sizeof(_word_t);
    RG[ra] = DMEM[RG[rsp] - sizeof(_word_t)];

    return 2;
}

int CPU_SEQ::ins_iadd(Instruction ins)
{
    int rb = ins[1] & 0xF;
    _sword_t a = RG[rb];
    _sword_t b = *(_word_t*)(&ins[2]);
    _sword_t result = a + b;

    // set CC
    if (a > 0 && b > 0 && result < 0 ||
        a < 0 && b < 0 && result > 0) {
        CC.OF = 1;
    } else {
        CC.OF = 0;
    }
    CC.SF = result < 0;
    CC.ZF = result == 0;

    RG[rb] = result;

    return 2 + sizeof(_word_t);
}

int CPU_SEQ::ins_null_handler(Instruction ins)
{
    Stat = SINS;
    return 0;
}

bool CPU_SEQ::calc_cnd(int ifun)
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

bool CPU_SEQ::addr_check(_word_t vaddr)
{
    if (vaddr > MSIZE) {
        Stat = SADR;
        return false;
    } else {
        return true;
    }
}