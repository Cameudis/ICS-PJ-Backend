#include <iostream>
using namespace std;

#include "CPU.h"

static CPU cpu;

int main()
{
    ifstream infile("test/prog1.yo");
    cpu.load_prog(infile);

    cpu.exec(114514);

    ofstream o("history.json");
    o << setw(4) << cpu.history << endl;

    return 0;
}