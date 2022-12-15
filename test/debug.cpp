#include "../src/include/json.hpp"
using json = nlohmann::json;

#include <bits/stdc++.h>
using namespace std;

json ans;
json my;

void print_info(int i)
{
    cout << "i: " << i << endl;
    cout << "my:" << endl;
    cout << setw(4) << my[i] << endl;
    cout << "answer:" << endl;
    cout << setw(4) << ans[i] << endl;
}

int main()
{
    ifstream ansin("case/asum.json");
    ansin >> ans;
    ansin.close();

    ifstream myin("h.json");
    myin >> my;
    myin.close();


    // compare
    if (ans.size() != my.size()) {
        cout << "size unmatch!";
    }
    for (int i = 0; i < ans.size(); i++) {
        if (ans[i] != my[i]) {
            if (ans[i]["MEM"] != my[i]["MEM"]) {
                cout << "MEM unmatch!\n";
            } else if (ans[i]["REG"] != my[i]["REG"]) {
                cout << "REG unmatch!\n";
            } else if (ans[i]["CC"] != my[i]["CC"]) {
                cout << "CC unmatch!\n";
            } else if (ans[i]["STAT"] != my[i]["STAT"]) {
                cout << "STAT unmatch!\n";
            } else {
                // only PC differ
                cout << "i: " << i << "\t";
                cout << "PC unmatch!" << endl;
                continue;
            }
            print_info(i);
            break;
        }
    }

    return 0;
}