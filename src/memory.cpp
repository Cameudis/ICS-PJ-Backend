#include "memory.h"

uint8_t* Memory::v2raddr(uint64_t vaddr)
{
    assert(vaddr <= MSIZE);
    return &mem[vaddr];
}

uint64_t Memory::r2vaddr(uint8_t* raddr) const
{
    return (uint64_t)raddr - (uint64_t)mem;
}

Instruction Memory::get_ins(uint64_t vaddr) const
{
    assert(vaddr <= MSIZE);
    return (Instruction)&mem[vaddr];
}

uint64_t& Memory::operator[](uint64_t vaddr)
{
    return (uint64_t&)(mem[vaddr]);
}