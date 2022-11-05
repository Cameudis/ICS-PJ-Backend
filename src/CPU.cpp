#include "CPU.h"

// ---------- init ----------

CPU::CPU()
{
    
}

void CPU::load_prog(std::ifstream& infile)
{

}

// ---------- exec ----------

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
    int icode = ins[0] & 0xF0;
    if (
        !icode ||                                       // rrmovq
        icode == 0x1 && ((CC.SF ^ CC.OF) | (CC.ZF)) ||  // cmovle
        icode == 0x2 && (CC.SF ^ CC.OF) ||              // cmovl
        icode == 0x3 && CC.ZF ||                        // cmove
        icode == 0x4 && !CC.ZF ||                       // cmovne
        icode == 0x5 && ~((CC.SF ^ CC.OF) | (CC.ZF)) || // cmovge
        icode == 0x6 && ~(CC.SF ^ CC.OF)                // cmovg
    ) {
        int ra = ins[1] & 0x0F;
        int rb = ins[1] & 0xF0;
        RG[rb] = RG[ra];
    }
    return 2;
}

int CPU::ins_irmov(Instruction ins)
{
    int rb = ins[1] & 0xF0;
    RG[rb] = *(word_t*)(&ins[2]);

    return 2 + sizeof(word_t);
}

int CPU::ins_rmmov(Instruction ins)
{
    int ra = ins[1] & 0x0F;
    int rb = ins[1] & 0xF0;
    word_t offset = *(word_t*)(&ins[2]);
    
    DMEM[offset + RG[rb]] = RG[ra];

    return 2 + sizeof(word_t);
}

int CPU::ins_mrmov(Instruction ins)
{
    int ra = ins[1] & 0x0F;
    int rb = ins[1] & 0xF0;
    word_t offset = *(word_t*)(&ins[2]);
    
    RG[ra] = DMEM[offset + RG[rb]];

    return 2 + sizeof(word_t);
}

int CPU::ins_op(Instruction ins)
{
    int icode = ins[0] & 0xF0;
    int ra = ins[1] & 0x0F;
    int rb = ins[1] & 0xF0;

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
    return 1 + sizeof(word_t);
}

int CPU::ins_call(Instruction ins)
{
    return 1 + sizeof(word_t);
}

int CPU::ins_ret(Instruction ins)
{
    return 1;
}

int CPU::ins_push(Instruction ins)
{
    return 2;
}

int CPU::ins_pop(Instruction ins)
{
    return 2;
}
