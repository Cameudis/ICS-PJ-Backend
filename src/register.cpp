#include "register.h"

uint64_t& reg::operator[](int id)
{
    assert(0x0 <= id && id <= 0xF);
    return regs[id];
}

const char* reg::get_reg_name(int id) const
{
    assert(0x0 <= id && id <= 0xF);
    return reg_name[id];
}