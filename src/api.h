// ---------- api usage ----------
// 1. invoke api functions
// 2. read updated state from file STAT_FILE_NAME ("crt_state.json")

#ifndef __API_H
#define __API_H

#include "CPU.h"

// ---------- api ----------

#define STAT_FILE_NAME "crt_state.json"

// give a <*.yo> file
extern "C" bool __cdecl api_load_prog(char* filename);

// exec <step> steps
extern "C" bool __cdecl api_step_exec(unsigned int step);

// exec instruction without update PC
extern "C" bool __cdecl api_imm_exec(int64_t part1, int64_t part2);

// time machine
extern "C" bool __cdecl api_revoke(int step);


// ---------- private ----------

static void output_crt_state();

#endif