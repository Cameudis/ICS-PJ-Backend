#include <iostream>
using namespace std;

#include "CPU.h"

static CPU cpu;

#ifndef NDEBUG
int debug()
{
    ifstream infile("D:\\NSS\\Project\\ICS-PJ-Backend\\test\\abs-asum-cmov.yo");
    cpu.load_prog(infile);
    cpu.exec(114514);
    cout << setw(4) << cpu.history << endl;

    exit(0);
}
#endif

int main()
{
    assert(debug());    // disappear when build binary

    cpu.load_prog(cin);
    cpu.exec(114514);
    cout << setw(4) << cpu.history << endl;

    return 0;
}