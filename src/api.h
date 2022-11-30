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

#define STAT_FILE_NAME "crt_state.json"

static MODE mode = INIT_MODE;

// mode switch (will reset all states)
extern "C" bool _DLLExport api_switch_mode(MODE to_mode);

// give a <*.yo> file
extern "C" bool _DLLExport api_load_prog(char* filename);

// current state interface
extern "C" bool _DLLExport api_get_state(bool* cc, int* stat, _word_t* pc, _word_t* reg, int8_t* mem);

// exec <step> steps
extern "C" bool _DLLExport api_step_exec(unsigned int step);

// exec instruction without update PC (only SEQ_MODE)
extern "C" bool _DLLExport api_imm_exec(int64_t part1, int64_t part2);

// time machine (only SEQ_MODE)
extern "C" bool _DLLExport api_revoke(int step);

// remake
extern "C" bool _DLLExport api_reset();

#endif