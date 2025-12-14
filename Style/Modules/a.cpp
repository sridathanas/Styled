#include "style.h"
using namespace std;


int main() {
    vector<string> commands = {
        "{ { ' ' }_100 }_40",
        "{ ' '_180 }_40",
    };

    int w = 188, h = 57;
    std::string s(h * (w + 1), ' ');
    for (int i = w; i < s.size(); i += (w + 1)) {
        s[i] = '\n';
    }

    int j = 0;
    while (true) {
        for (auto it = colors.begin(); it != colors.end(); ++it) {
            cout << "\033[H\033[48;2;" + it->second + "m";
            if (++j == 100000)
                return 0;
            cout << s << flush;
            system("pause > nul");
        }
    }
}


/*
i: 0
{ agg@:: $arr$i }_<3>
{
    ?<$i %% 2> t3.
    ?<> t.3  <>

    bg:$it H
    & (' '_$SCW + "`n")_$SCH

}_<$arr1 + $arr2>
*/