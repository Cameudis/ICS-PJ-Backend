#include <iostream>
using namespace std;

#include "CPU.h"
// static CPU cpu;

#ifndef NDEBUG
#include "api.h"
int debug()
{
    api_load_prog("test/prog1.yo");
    api_step_exec(2);
    api_revoke(1);

    exit(0);
}
#endif

int main()
{
    assert(debug());    // disappear when build binary

    // cpu.load_prog(cin);
    // cpu.exec(114514);
    // cout << setw(4) << cpu.history << endl;

    return 0;
}