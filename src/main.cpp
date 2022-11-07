#include <iostream>
using namespace std;

#include "CPU.h"

#ifndef NDEBUG
#include "api.h"
int debug()
{
    api_load_prog("test/prog1.yo");
    api_step_exec(3);
    api_revoke(2);

    exit(0);
}
#endif

static CPU cpu;

int main()
{
    assert(debug());    // disappear when build binary

    cpu.load_prog(cin);
    cpu.exec(114514);
    cout << setw(4) << cpu.history[cpu.history.size()-1] << endl;

    return 0;
}