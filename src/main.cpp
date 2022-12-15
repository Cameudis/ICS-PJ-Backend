#include <iostream>
using namespace std;

#include "CPU_PIPE.h"
#include "CPU_SEQ.h"
static CPU* cpu;

// #ifndef NDEBUG
// #endif

int main(int argc, char* argv[])
{
    if (argc == 1) {
        cpu = new CPU_PIPE;
    } else {
        if (!strcmp(argv[1], "SEQ")) {
            cpu = new CPU_SEQ;
        } else {
            cpu = new CPU_PIPE;
        }
    }

    cpu->load_prog(cin);
    cpu->exec(114514);
    cpu->print_history();

    return 0;
}