#include "common.h"

class Register {
public:
    // reload [] to get 64-bit value by vaddr
    uint64_t& operator[](int id);

    const char* get_reg_name(int id) const;

private:
    uint64_t regs[16];
    const char* reg_name[16] = {
        "rax",  "rcx",  "rdx",  "rbx",
        "rsp",  "rbp",  "rsi",  "rdi",
        "r8",   "r9",   "r10",  "r11",
        "r12",  "r13",  "r14",  "NULL"
    };
};