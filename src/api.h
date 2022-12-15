// ---------- api usage ----------
// 0. set CPU mode
// 1. invoke api functions
// 2. read updated state via api_get_state()

#ifndef __API_H
#define __API_H

#include "CPU_SEQ.h"
#include "CPU_PIPE.h"
#define _DLLExport __declspec (dllexport)

// ---------- api ----------

enum MODE {
    SEQ_MODE,
    PIPE_MODE
};
static MODE mode = INIT_MODE;

// mode switch (will reset all states)
// return false when fail to create CPU instance (shouldn't happen)
extern "C" bool _DLLExport api_switch_mode(MODE to_mode);

// give a <*.yo> file
// return false when fail to create CPU instance (shouldn't happen)
extern "C" bool _DLLExport api_load_prog(char* filename);

// current state interface
// always return true
extern "C" bool _DLLExport api_get_state(bool* cc, int* stat, _word_t * pc, _word_t * reg, int8_t * mem);

// pipeline register information interface (PIPE_MODE only)
// return false if mode != PIPE
extern "C" bool _DLLExport api_get_PRstate(char* fetch, char* decode, char* execute, char* memory, char* writeback);

// exec <step> steps
// return false when progress HALT (usually Exception or execute complete)
extern "C" bool _DLLExport api_step_exec(unsigned int step);

// exec instruction without update PC (SEQ_MODE only)
// return false if mode == PIPE
extern "C" bool _DLLExport api_imm_exec(int64_t part1, int64_t part2);

// time machine (SEQ_MODE only)
// return false if mode == PIPE or back too much steps
extern "C" bool _DLLExport api_revoke(int step);

// remake
// always return true
extern "C" bool _DLLExport api_reset();

#endif