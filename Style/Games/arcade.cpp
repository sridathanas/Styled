#include "../Modules/style.h"


const Coord TOP_LC = { 16, 2 };
const Coord TOP_RC = { 16, Scr::WIDTH - 2 };
const Coord BOT_LC = { Scr::HEIGHT - 1, 2 };
const Coord BOT_RC = { Scr::HEIGHT - 1, Scr::WIDTH - 2 };


namespace Pong
{
    //BOUNDARIES
    const std::string WALL_COLOR = "BLUE";


    //PAD
    int PAD_HT = 7;  // height
    const int PAD_WD = 2;  // width
    const std::string PAD_FG = Scr::SCREEN_BG;
    const std::string PAD_BG = "PURPLE";

    Coord PAD_LC = TOP_RC + Coord(1, 0);  // pad left corner


    //BALL
    const Coord BALL_ST = { 18, TOP_LC.COL + 2 };  // ball start position
    const std::vector<Coord> BALL_PTS = {
        BALL_ST,
        BALL_ST + Coord(-1, 1),
        BALL_ST + Coord(0, 1),
        BALL_ST + Coord(1, 1),
        BALL_ST + Coord(0, 2),
        BALL_ST + Coord(-1, 0),
        BALL_ST + Coord(1, 0),
        BALL_ST + Coord(-1, 2),
        BALL_ST + Coord(1, 2)
    };

    std::string BALL_FG = Scr::SCREEN_BG;
    std::string BALL_BG = "BLUE";
    Coord BALL_DIR = { 1, 1 };  // ball will move down by default


    bool MovePad(Block& pad, int where)
    {
        if (where == Dir::UP)
        {
            if (PAD_LC.ROW == TOP_RC.ROW + 1)
                return false;
            
            pad.MoveBy({ -1, 0 });
            PAD_LC.ROW--;
        }
        else if (where == Dir::DOWN)
        {
            if (PAD_LC.ROW + PAD_HT == BOT_RC.ROW)
                return false;

            pad.MoveBy({ 1, 0 });
            PAD_LC.ROW++;
        }

        return true;
    }


    bool ProcessMove(Block& ball, Block& pad)
    {
        // top and bottom walls
        if (BALL_DIR.ROW == 1 && ball.Collides(BOT_LC, "horz") || BALL_DIR.ROW == -1 && ball.Collides(TOP_LC, "horz"))
            BALL_DIR.ROW *= -1;

        // left wall
        else if (BALL_DIR.COL == -1 && ball.Collides(TOP_LC, "vert"))
            BALL_DIR.COL *= -1;

        // right-side
        else if (BALL_DIR.COL == 1 && ball.Collides(TOP_RC, "vert"))
        {
            if (ball.Collides(pad))
                BALL_DIR.COL *= -1;
            else
                return true;
        }

        return false;
    }


    bool MoveBall(Block& ball, Block& pad)
    {
        ball.MoveBy(BALL_DIR);
        return !ProcessMove(ball, pad);
    }
}


void PingPong(bool naughty = false)
{
    Scr::SetStyle("H");

    if (naughty)
    {
        Scr::Paint("CYAN", TOP_LC, BOT_RC);
        Scr::SCREEN_BG = "RED";
        Pong::PAD_HT = 25;
    }
    else {
        Pong::PAD_HT = 7;
    }

    // CREATING THE WALLS
    Scr::SetColor(Pong::WALL_COLOR);
    Figure::Fill(TOP_LC, TOP_RC, '-');
    Figure::Fill(BOT_LC, BOT_RC, '-');
    Figure::Fill(TOP_LC, BOT_LC, '|');
    Scr::Puts("+", TOP_LC);
    Scr::Puts("+", BOT_LC);


    // MAKING THE PAD
    Block pad(" ", Pong::PAD_WD, Pong::PAD_HT, Pong::PAD_LC, Pong::PAD_BG);
    pad.ChangeColor(Pong::PAD_FG, Pong::PAD_BG);


    // MAKING THE BALL
    Block ball(" ", 4, 3, Pong::BALL_ST, Pong::BALL_BG);
    ball.ChangeColor(Pong::BALL_FG, Pong::BALL_BG);


    // MOVING THE BALL, COLLISIONS
    while (Pong::MoveBall(ball, pad))
    {
        // change i to increase speed of pad
        for (int i = (naughty ? 1 : 2); i--; )
        {
            if (GetAsyncKeyState(VK_UP) & 0x8000)
                Pong::MovePad(pad, Dir::UP);

            if (GetAsyncKeyState(VK_DOWN) & 0x8000)
                Pong::MovePad(pad, Dir::DOWN);

            if (GetAsyncKeyState(VK_BACK) & 0x8000)
                return;
        }
        
        outBuff.flush();
        MicroSleep((naughty ? 3 : 30) MS);
    }
}


int main()
{
    Scr::Paint("DARKCYAN", {1, 1}, {10, -1});
    CanvasDraw("(4 .47) H B PARROT \"GAME ZONE\" d2 (0 .45) U RED \"RESPONSIVE PAGE!\" UU UB");

    Grid menu({11, 1}, "", "ROSE", ' ', ' ', ' ');
    
    menu.skipEnds = true;
    menu.Configure(1, 4, 3, 45, "vert");

    menu.Update({1, 1}, "Ping-pong", "", "GRAY", "B");
    menu.Update({1, 2}, "Poopdye", "PURPLE", "GRAY", "B");
    menu.Update({1, 3}, "Help", "GREEN", "GRAY", "B");
    menu.Update({1, 4}, "Next", "BATHROOM", "GRAY", "B");
    

    int counter = 1, wait = 0, select = 1;

    Block boulder(" ", Scr::WIDTH - 2, 33, TOP_LC, "", "BLACK");
    boulder.Clear();

    std::string prevBg = menu.data[select - 1].style.bgColor;
    std::string prevFg = menu.data[select - 1].style.fgColor;
    std::string bgHighlight = "BLUE", fgHighlight = "CYAN";
    menu.Update({1, 1}, "", fgHighlight, bgHighlight);
    menu.Draw();


    while (true)
    {
        if (!wait)
        {
            if (select > 1 && GetAsyncKeyState(VK_LEFT) & 0x8000)
            {
                menu.Update({1, select}, "", prevFg, prevBg);
                select--;
                prevBg = menu.data[select-1].style.bgColor;
                prevFg = menu.data[select-1].style.fgColor;
                menu.Update({1, select}, "", fgHighlight, bgHighlight);
                menu.Draw();
                
                wait = 5;
            }

            else if (select < 4 && GetAsyncKeyState(VK_RIGHT) & 0x8000)
            {
                menu.Update({1, select}, "", prevFg, prevBg);
                select++;
                prevBg = menu.data[select-1].style.bgColor;
                prevFg = menu.data[select-1].style.fgColor;
                menu.Update({1, select}, "", fgHighlight, bgHighlight);
                menu.Draw();
                
                wait = 5;
            }

            else if (GetAsyncKeyState(VK_RETURN) & 0x8000)
            {
                if (select == 1) {
                    Scr::Paint("BLACK", TOP_LC, BOT_RC);
                    Scr::SCREEN_BG = "BLACK";
                    PingPong();
                }
                else if (select == 2) {
                    boulder.ChangeColor("WHITE", "POOP");
                    boulder.ChangeSeq("POOPDYE ");
                }
                else if (select == 3) {
                    boulder.ChangeColor("BLACK", "CYAN");
                    boulder.ChangeSeq("? ");
                }
                else {
                    PingPong(true);
                }

                wait = 5;
            }
        }
        else {
            wait--;
        }

        if (counter++ == 50) {
            counter = 1;
        }

        outBuff.flush();
        MicroSleep(40 MS);
    }

    Pause(1 M);
}