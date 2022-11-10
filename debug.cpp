#include "src/include/json.hpp"
using json = nlohmann::json;

#include <bits/stdc++.h>
using namespace std;

int main()
{
    ifstream ansin("test/asumi.json");
    json ans;
    ansin >> ans;
    ansin.close();

    ifstream myin("h.json");
    json my;
    myin >> my;
    myin.close();


    // compare
    if (ans.size() != my.size()) {
        cout << "size unmatch!";
    }
    for (int i = 0; i < ans.size(); i++) {
        if (ans[i] != my[i]) {
            cout << "i: " << i << endl;
            cout << "my:" << endl;
            cout << setw(4) << my[i];
            cout << "answer:" << endl;
            cout << setw(4) << ans[i];

            break;
        }
    }

    return 0;
}