#include "memory.h"

uint8_t* memory::v2raddr(uint64_t vaddr)
{
    assert(vaddr <= MSIZE);
    return &mem[vaddr];
}

uint64_t memory::r2vaddr(uint8_t* raddr) const
{
    return (uint64_t)raddr - (uint64_t)mem;
}

uint64_t& memory::operator[](uint64_t vaddr)
{
    return (uint64_t&)(mem[vaddr]);
}