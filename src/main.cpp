#include <cstdio>
using namespace std;

#include "CPU.h"

static CPU cpu;

int main()
{
    ifstream infile("test/prog1.yo");
    cpu.load_prog(infile);

    return 0;
}