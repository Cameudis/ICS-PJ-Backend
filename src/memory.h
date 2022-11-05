#ifndef __MEMORY_H
#define __MEMORY_H

#include "common.h"

class Memory {
public:
    // transform between vaddr and raddr
    uint8_t* v2raddr(word_t vaddr);
    word_t r2vaddr(uint8_t* raddr) const;

    // reload [] to get 64-bit value by vaddr
    word_t& operator[](word_t vaddr);

    // get instruction
    Instruction get_ins(word_t vaddr);

private:
    // memory
    uint8_t mem[MSIZE] = {};
};

#endif