#include "Register.h"

word_t& Register::operator[](int id)
{
    assert(0x0 <= id && id <= 0xF);
    return regs[id];
}

const char* Register::get_reg_name(int id) const
{
    assert(0x0 <= id && id <= 0xF);
    return reg_name[id];
}