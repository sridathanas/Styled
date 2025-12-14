#include "..\Modules\style.h"


//BOUNDARIES
const Coord TOP_LC = { 2, 3 };  // top left corner
const Coord TOP_RC = { 2, Scr::WIDTH - 3 };  //immediately at and beyond this padding level, the game ends
const Coord BOT_LC = { Scr::HEIGHT - 3, 3 };
const Coord BOT_RC = { Scr::HEIGHT - 3, Scr::WIDTH - 3 };
const std::string WALL_COLOR = "RED";


//PAD
const int PAD_HT = 10;  // height
const int PAD_WD = 2;  // width
const std::string PAD_FG = Scr::SCREEN_BG;
const std::string PAD_BG = "CYAN";

Coord PAD_LC = { (TOP_RC.ROW + BOT_RC.ROW - PAD_HT ) / 2 + 1, TOP_RC.COL };  // pad left corner


//BALL
const Coord BALL_ST = { (3 * TOP_RC.ROW + BOT_RC.ROW) / 2, TOP_LC.COL + 2 };  // ball start position
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
std::string BALL_BG = "RED";
Coord BALL_DIR = { 1, 1 };  // ball will move down by default


// returns false if pad stuck
bool MovePad(Block& pad, int where);

// collision detection/bouncing
bool ProcessMove(Block& ball, Block& pad);

// moves ball one step, return false if ball has collided
bool MoveBall(Block& ball, Block& pad);

void pause(int x = 1) { outBuff.flush(); MicroSleep(x S); }



int main()
{
    Scr::SetStyle("H");
    Scr::Paint("BLACK");


    // CREATING THE WALLS
    Scr::SetColor(WALL_COLOR);
    Figure::Fill(TOP_LC, TOP_RC, '-');
    Figure::Fill(BOT_LC, BOT_RC, '-');
    Figure::Fill(TOP_LC, BOT_LC, '|');
    Scr::Puts("+", TOP_LC);
    Scr::Puts("+", BOT_LC);


    // MAKING THE PAD
    Block pad(" ", PAD_WD, PAD_HT, PAD_LC, PAD_BG);
    pad.ChangeColor(PAD_FG, PAD_BG);


    // MAKING THE BALL
    Block ball(" ", 4, 3, BALL_ST, BALL_BG);
    ball.ChangeColor(BALL_FG, BALL_BG);
    //ball.ChangeColor(BALL_FG, BALL_BG);


    // MOVING THE BALL, COLLISIONS
    while (MoveBall(ball, pad))
    {
        // change i to increase speed of pad
        for (int i = 0; i < 2; i++)
        {
            if (GetAsyncKeyState(VK_UP) & 0x8000)
                MovePad(pad, Dir::UP);

            if (GetAsyncKeyState(VK_DOWN) & 0x8000)
                MovePad(pad, Dir::DOWN);
        }
        
        outBuff.flush();
        MicroSleep(30 MS);
    }


    Scr::Paint("BLUE");
    Pause();
}


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