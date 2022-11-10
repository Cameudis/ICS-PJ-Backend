#include "memory.h"

uint8_t* Memory::v2raddr(_word_t vaddr)
{
    assert(vaddr <= MSIZE);
    return &mem[vaddr];
}

_word_t Memory::r2vaddr(uint8_t* raddr) const
{
    return (_word_t)raddr - (_word_t)mem;
}

Instruction Memory::get_ins(_word_t vaddr)
{
    assert(vaddr <= MSIZE);
    return &mem[vaddr];
}

_word_t& Memory::operator[](_word_t vaddr)
{
    return (_word_t&)(mem[vaddr]);
}