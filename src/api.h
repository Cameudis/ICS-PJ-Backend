// ---------- api usage ----------
// 1. invoke api functions
// 2. read updated state from file STAT_FILE_NAME ("crt_state.json")

#ifndef __API_H
#define __API_H

#include "CPU.h"
#define _DLLExport __declspec (dllexport)

// ---------- api ----------

#define STAT_FILE_NAME "crt_state.json"

// give a <*.yo> file
extern "C" bool _DLLExport api_load_prog(char* filename);

// exec <step> steps
extern "C" bool _DLLExport api_step_exec(unsigned int step);

// exec instruction without update PC
extern "C" bool _DLLExport api_imm_exec(int64_t part1, int64_t part2);

// time machine
extern "C" bool _DLLExport api_revoke(int step);

// remake
extern "C" bool _DLLExport api_reset();


// ---------- private ----------

static void output_crt_state();

#endif