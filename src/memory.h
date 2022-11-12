#ifndef __MEMORY_H
#define __MEMORY_H

#include "common.h"
#include <cstring>

class Memory {
public:
    // transform between vaddr and raddr
    uint8_t* v2raddr(_word_t vaddr);
    _word_t r2vaddr(uint8_t* raddr) const;

    // reload [] to get 64-bit value by vaddr
    _word_t& operator[](_word_t vaddr);

    // get instruction
    Instruction get_ins(_word_t vaddr);

    // clear
    void clear() { memset(mem, 0, MSIZE); }

private:
    // memory
    uint8_t mem[MSIZE] = {};
};

#endif