#include "memory.h"

uint8_t* Memory::v2raddr(word_t vaddr)
{
    assert(vaddr <= MSIZE);
    return &mem[vaddr];
}

word_t Memory::r2vaddr(uint8_t* raddr) const
{
    return (word_t)raddr - (word_t)mem;
}

Instruction Memory::get_ins(word_t vaddr)
{
    assert(vaddr <= MSIZE);
    return &mem[vaddr];
}

word_t& Memory::operator[](word_t vaddr)
{
    return (word_t&)(mem[vaddr]);
}