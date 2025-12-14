#include "../Modules/style.h"


int main() {
    Grid grid({ 55, 45 });
    grid.Configure(1, 1, 3, 100);
    Scr::Paint("BLACK");


    while (true) {
        Scr::SaveState();
        std::string cmd = Entry::TakeInput(grid, { 1, 1 }, "", "CYAN", "%10;0;0");
        Scr::RetrieveState();
        CanvasDraw(cmd);
    }
}