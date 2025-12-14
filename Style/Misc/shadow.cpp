#include "../Modules/style.h"


int main()
{
    Coord topLeft(20, 40);
    int width = 40, height = 10;

    Block shadow(" ", width, height, topLeft + Coord(1, 2));
    Block block(" ", width, height, topLeft, "PINK");

    // ################################################

    #define IS_VALID(n) ((n) >= 0 && (n) <= 255)
    int R = 0, G = 0, B = 0;

    const Coord GRID_TL = { 10, 160 };
    Grid b(GRID_TL);

    b.Configure(3, 1, 6, 15, "none");
    b.Update({1, 1}, "RED", "", "RED");
    b.Update({2, 1}, "GREEN", "", "EMERALD");
    b.Update({3, 1}, "BLUE", "", "BLUE");
    b.Draw();

    Button btn(&b, State(Coord(), "WHITE", "BLACK"), 1, 30);

    while (true)
    {
        Coord coord = btn.Check();
        int delta = 0;
        if (coord.isValid())
            delta = 1;
        else if (coord == Button::BACKSPACE)
            delta = -1;
        
        if (btn.selCoord == Coord(1, 1) && IS_VALID(R+delta))
            R += delta;
        else if (btn.selCoord == Coord(2, 1) && IS_VALID(G+delta))
            G += delta;
        else if (btn.selCoord == Coord(3, 1) && IS_VALID(B+delta))
            B += delta;

        auto color = "%" + std::to_string(R) + ";" + std::to_string(R) + ";" + std::to_string(R);

        // here here
        shadow.ChangeColor("", color);
        Block block(" ", width, height, topLeft, "PINK");

        b.Render();
        Scr::SetColor("WHITE", "CONSOLE");
        Scr::Puts(color + " ", GRID_TL + Coord(20, 1));

        Pause(50 ms);
    }

// ################################################
    Pause();
}