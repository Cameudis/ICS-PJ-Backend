#include <iostream>
using namespace std;

#include "CPU_PIPE.h"
static CPU cpu;

// #ifndef NDEBUG
// #include "api.h"
// void print_state()
// {
//     ifstream inf("crt_state.json");
//     std::string str((std::istreambuf_iterator<char>(inf)),
//                  std::istreambuf_iterator<char>());
//     cout << str << endl << endl;
// }
// int debug()
// {
//     api_load_prog("case/prog1.yo");
//     char ch;
//     while((ch = getchar()) == '\n') {
//         api_step_exec(1);
//         print_state();
//     }
//     exit(0);
// }
// #endif

int main()
{
    // assert(debug());    // disappear when build binary

    cpu.load_prog(cin);
    cpu.exec(500);
    cpu.print_history();

    return 0;
}