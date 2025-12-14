#include "..\..\Modules\style.h"


int main()
{
    Scr::Paint("BLACK");
    outBuff.bufferSize = 500;
    double rgb = 255;

    int counter = 10000;
    int barWidth = 1;

    for (int i = 0; true; i++)
    {
        Coord coord = {Ranint(1, Scr::HEIGHT), Ranint(1, Scr::WIDTH)};
        Scr::AtCoord(coord);

        Scr::SetColor("", ColorSweep());
        int len = (Scr::WIDTH - coord.COL <= barWidth) ? Scr::WIDTH - coord.COL : barWidth;

        outBuff << std::string(len, ' ');
    }
}