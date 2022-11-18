#include <fstream>
#include "api.h"
using std::ifstream;
using std::ofstream;

static CPU cpu;

extern "C" bool _DLLExport api_load_prog(char* filename)
{
    ifstream ifd(filename);
    cpu.load_prog(ifd);

    return true;
}

extern "C" bool _DLLExport api_step_exec(unsigned int step)
{
    cpu.exec(step);
    output_crt_state();

    return (cpu.history[cpu.history.size()-1]["STAT"]) == SAOK;
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

    cpu.im_exec(ins);
    output_crt_state();
    return true;
}

extern "C" bool _DLLExport api_revoke(int step)
{
    if (cpu.back(step)) {
        output_crt_state();
        return true;
    } else {
        return false;
    }
}

extern "C" bool _DLLExport api_reset()
{
    cpu.reset();
    return true;
}

static void output_crt_state()
{
    ofstream ofd(STAT_FILE_NAME);
    ofd << cpu.history[cpu.history.size()-1];
    ofd.close();
}