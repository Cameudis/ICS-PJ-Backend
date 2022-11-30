#include <fstream>
#include "api.h"
using std::ifstream;
using std::ofstream;

static CPU *cpu;

extern "C" bool _DLLExport api_switch_mode(MODE to_mode)
{
    delete cpu;
    if (to_mode == SEQ_MODE) {
        cpu = new CPU_SEQ;
    } else if (to_mode == PIPE_MODE) {
        cpu = new CPU_PIPE;
    }
}

extern "C" bool _DLLExport api_load_prog(char* filename)
{
    if (!cpu) {
        api_switch_mode(mode);
    }
    ifstream ifd(filename);
    if (mode == SEQ_MODE)
        cpu->load_prog(ifd);

    return true;
}

extern "C" bool _DLLExport api_get_state(bool* cc, int* stat, _word_t* pc, _word_t* reg, int8_t* mem)
{
    return cpu->get_state(cc, stat, pc, reg, mem);
}

extern "C" bool _DLLExport api_step_exec(unsigned int step)
{
    if (!cpu) {
        api_switch_mode(mode);
    }
    cpu->exec(step);
    return cpu->is_SAOK();
}

extern "C" bool _DLLExport api_imm_exec(int64_t part1, int64_t part2)
{
    uint8_t ins[10] = {};
    uint8_t *p1 = (uint8_t *)&part1;
    uint8_t *p2 = (uint8_t *)&part2;

    int i = 0;
    ins[i++] = p1[0];
    if (part1 >> 8) {
        ins[i++] = p1[1];
    }
    *(int64_t*)&ins[i] = part2;

    cpu->im_exec(ins);
    return true;
}

extern "C" bool _DLLExport api_revoke(int step)
{
    if (cpu->back(step)) {
        return true;
    } else {
        return false;
    }
}

extern "C" bool _DLLExport api_reset()
{
    cpu->reset();
    return true;
}
