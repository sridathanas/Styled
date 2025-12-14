#include "..\..\Modules\style.h"
#include <ctime>


void foo(int step, int n = 1000, float wait = 1, char color = 'r')
{
    Scr::Puts(Fmt("STEP = %d", step), {1, Scr::WIDTH / 2});
    step *= -1;

    int row = 2, col = 1, colIncr = 3, rowIncr = 1;

    int len = HorzLine::MonoBgGradient(200, 100, step);
    srand(time(0));
    while (true)
    {
        HorzLine::MonoBgGradient({row, col}, 200, 100, (color != 'a') ? color : "rgb"[rand() % 3], step);
        col += colIncr;
        row += rowIncr;
        if (col + len >= Scr::WIDTH + 1600 && colIncr > 0 || col <= 1 && colIncr < 0)
            colIncr *= -1;
        if (row == Scr::HEIGHT + 450 || row == 2)
            rowIncr *= -1;
        if (wait)
            Pause(wait MS);
    }
}


int main()
{
    // foo(7, 1500, 2);
    // foo(10, 2000, 1);
    // foo(16, 5000, 0.9);
    // foo(20, 4000, 1, 'r');
    // foo(25, 5000, 0.8, 'a');
    // foo(30, -1, 0.05, 'a');
    foo(9, -1, 0, 'r');
}