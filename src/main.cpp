#include <iostream>
using namespace std;

#include "CPU.h"

static CPU cpu;

int main()
{
    cpu.load_prog(cin);
    cpu.exec(114514);
    cout << setw(4) << cpu.history << endl;

    return 0;
}