#include "common.h"

class memory {
public:
    // transform between vaddr and raddr
    uint8_t* v2raddr(uint64_t vaddr);
    uint64_t r2vaddr(uint8_t* raddr) const;

    // reload [] to get 64-bit value by vaddr
    uint64_t& operator[](uint64_t vaddr);

private:
    // memory
    uint8_t mem[MSIZE] = {};
};