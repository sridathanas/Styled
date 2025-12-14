#include "..\Modules\style.h"
#include <ctime>


// Boundary
const Coord TOP_LC = { 2, 3 };
const Coord BOT_LC = { Scr::HEIGHT - TOP_LC.ROW, TOP_LC.COL };
const Coord TOP_RC = { TOP_LC.ROW, Scr::WIDTH - TOP_LC.COL };
const Coord BOT_RC = { Scr::HEIGHT - TOP_LC.ROW, Scr::WIDTH - TOP_LC.COL };


// Tank
const int TANK_W = 8;
const int TANK_H = 20;
const std::string TANK_COLOR = "OCHRE";


// Shooter
const Coord SHOOT_TIP = BOT_LC + Coord(-3, 4);
const Coord SHOOT_MAP[9] = {
    SHOOT_TIP,
    SHOOT_TIP + Coord(1, -1),
    SHOOT_TIP + Coord(1, 0),
    SHOOT_TIP + Coord(1, 1),
    SHOOT_TIP + Coord(2, -2),
    SHOOT_TIP + Coord(2, -1),
    SHOOT_TIP + Coord(2, 0),
    SHOOT_TIP + Coord(2, 1),
    SHOOT_TIP + Coord(2, 2)
};
const std::string SHOOT_COLOR = "PARROT";


// Fish
Coord FISH_TOP = { TOP_LC.ROW + TANK_H + 3, (TOP_LC.COL + TOP_RC.COL) / 2 };


class Fish
{
    private:
        std::string color;
        Group* elements;
        int hitPoints;


    public:
        int freq, width, height;
        Coord dir = { 0, 1 };
        Coord vertex;

        Fish(const Coord& vertex, int freq = 4, int hitPoints = 1, int width = 9, int height = 2, std::string color = "CYAN")
            : width(width), height(height), color(color), freq(freq), vertex(vertex), hitPoints(hitPoints)
        {
            std::vector<Figure*> points;

            for (int i = 0; i < height; i++)
            {
                points.push_back(new Point('+', vertex + Coord(i, 0), color, ""));
                points.push_back(new Point('+', vertex + Coord(i, width - 1), color, ""));
            }

            for (int i = 1; i < width - 1; i++) {
                points.push_back(new Point('-', vertex + Coord(0, i), color, ""));
            }

            for (int i = 1; i < width - 1; i++)
            {
                char c = (i == (width - 1) / 2) ? '.' : '-';
                points.push_back(new Point(c, vertex + Coord(height - 1, i), color, ""));
            }

            elements = new Group(points);
        }


        void Move()
        {
            if (dir.COL == 1 && elements->Collides(TOP_RC, "all") || dir.COL == -1 && elements->Collides(TOP_LC, "all")) {
                dir = { 0, -dir.COL };
            }

            elements->MoveBy(dir);
            vertex.COL += dir.COL;
        }


        bool DealHit(int damage)
        {
            Scr::SaveState();

            for (auto it = colors.begin(); it != colors.end(); it++)
            {
                elements->ChangeColor(it->first);
                Pause(20 MS);
            }
            if ((hitPoints -= damage) <= 0)
            {
                elements->Clear(false);
                Scr::RetrieveState();
                return true;
            }
            else if (hitPoints == 1) {
                elements->ChangeColor("RED");
            }
                      
            Scr::RetrieveState();
            return false;
        }


        bool IsAlive()
        {
            return hitPoints > 0;
        }


        Coord getBottom()
        {
            return { vertex.ROW + height - 1, vertex.COL + (width - 1) / 2 };
        }


        ~Fish()
        {
            elements->Clear();
        }
};


void MakeTank(const Coord& where);
int RandInt(int st, int end);
int RandInt(int pool[], int length);


int main()
{
    Scr::Paint("BLACK");
    Scr::SetStyle("H");
    
    // Tank
    MakeTank(TOP_LC);
    MakeTank(TOP_RC - Coord(0, TANK_W - 1));

    
    // Poop
    Block poop1(" ", TANK_W - 2, 1, TOP_LC + Coord(TANK_H - 2, 1), "OCHRE");
    Block poop2(" ", TANK_W - 2, 1, TOP_RC + Coord(TANK_H - 2, -TANK_W+2), "OCHRE");
    int stinkLevel = 0;
    bool wormDance = false;


    // Wall
    Scr::SetColor("WHITE", Scr::SCREEN_BG);
    Figure::Fill(TOP_LC + Coord(TANK_H, 0), BOT_LC, '+');
    Figure::Fill(TOP_RC + Coord(TANK_H, 0), BOT_RC, '+');
    Figure::Fill(BOT_LC, BOT_RC, '=');


    // Shooter
    std::vector<Figure*> points;
    for (const auto& coord : SHOOT_MAP) {
        points.push_back(new Point('*', coord, SHOOT_COLOR));
    }
    Group shooter(points);


    // Bullet
    std::vector<Point*> bullets;


    // Boulder
    Block boulder(" ", TOP_RC.COL - TOP_LC.COL - 2 * TANK_W + 1, TANK_H, TOP_LC + Coord(1, TANK_W), "GRAY");
    Coord boulderDir = { 1, 0 };


    // Fish
    std::srand(time(0));

    int freqPool[] = { 3, 4, 6, 8, 12, 24, 3, 1 };
    std::vector<Fish*> fishes = {
        new Fish(FISH_TOP, RandInt(freqPool, 7), RandInt(3, 5), 9, 2, "BLUE"),
        new Fish(FISH_TOP + Coord(3, 0), RandInt(freqPool, 7), RandInt(2, 5), 9, 2, "CYAN"),
        new Fish(FISH_TOP + Coord(6, 0), RandInt(freqPool, 7), RandInt(1, 5), 9, 2, "PINK")
    };


    // Movement
    int counter = 1;

    while (true)
    {
        if (!shooter.Collides(TOP_RC, "vert", 2) && GetAsyncKeyState(VK_RIGHT) & 0x8000) {
            shooter.MoveBy({ 0, 1 });
        }
        else if (!shooter.Collides(TOP_LC, "vert", 2) && GetAsyncKeyState(VK_LEFT) & 0x8000) {
            shooter.MoveBy({ 0, -1 });
        }
        else if (counter % 12 == 0 && GetAsyncKeyState(VK_RETURN) & 0x8000)
        {
            auto elms = shooter.GetElements();
            bullets.push_back(new Point('|', elms[0]->thisState.coord + Coord(-1, 0), "PINK", ""));
        }
        else if (counter % 48 == 0 && GetAsyncKeyState(VK_SUBTRACT) & 0x8000)
        {
            auto it = colors.begin();
            while (true)
            {
                shooter.ChangeColor(it->first);
                if (++it == colors.end())
                    break;
                Pause(15 MS);
            }
            auto elms = shooter.GetElements();
            bullets.push_back(new Point('&', elms[0]->thisState.coord + Coord(-1, 0), "RED", ""));
        }


        if (counter % 4 == 0 && !bullets.empty())
        {
            for (auto bullet : bullets)
            {
                bullet->MoveBy({ -1, 0 });
                Coord bulletCoord = bullet->thisState.coord;

                if (bulletCoord.ROW < FISH_TOP.ROW)
                {
                    if (bulletCoord.ROW <= boulder.thisState.coord.ROW + boulder.height + 1 || bulletCoord.ROW <= TOP_LC.ROW)
                    {
                        bullet->Clear();
                        bullets.erase(std::remove(bullets.begin(), bullets.end(), bullet), bullets.end());
                        delete bullet;
                    }
                }
                else
                {
                    FISH_TOP = BOT_LC;
                    for (auto fish : fishes)
                    {
                        if (!fish->IsAlive()) continue;

                        Coord bottom = fish->getBottom();
                        bool isSpecialBullet = bullet->thisState.fgColor == "RED";
                        if (bulletCoord == bottom + Coord(1, 0) || isSpecialBullet && bulletCoord.ROW == bottom.ROW + 1 && abs(bulletCoord.COL - bottom.COL) <= fish->width / 2)
                        {
                            if (fish->DealHit(isSpecialBullet ? 3 : 1))
                            {
                                fishes.erase(std::remove(fishes.begin(), fishes.end(), fish), fishes.end());
                            }

                            bullet->Clear();
                            bullets.erase(std::remove(bullets.begin(), bullets.end(), bullet), bullets.end());
                            delete bullet;
                        }
                        if (fish->IsAlive() && fish->vertex.ROW < FISH_TOP.ROW) {
                            FISH_TOP = fish->vertex;
                        }
                    }
                }
            }
        }

        if (counter % 8 == 0)
        {
            if (!boulder.Reframe(boulderDir) || boulder.Collides(FISH_TOP, "horz"))
            {
                boulderDir = { -boulderDir.ROW, boulderDir.COL };
            }
            if (boulder.Collides(SHOOT_TIP, "horz", 2)) {
                shooter.Clear();
                boulder.Clear();
                poop1.Clear();
                poop2.Clear();
                
                Scr::SetColor("OCHRE", "POOP");
                Figure::Fill(TOP_LC + Coord(TANK_H, 0), BOT_LC, '+');
                Figure::Fill(TOP_RC + Coord(TANK_H, 0), BOT_RC, '+');
                Figure::Fill(BOT_LC, BOT_RC, '=');
                break;
            }
        }

        for (auto fish : fishes)
        {
            if (counter % fish->freq == 0) {
                fish->Move();
            }
        }

        if (++counter == 241) {
            if (stinkLevel > 2)
            {
                MicroSleep(2 S);
                Scr::Paint("RED");
                return -1;
            }

            poop1.MoveBy({ -1, 0 });
            poop1.Reframe({ 1, 0 });

            poop2.MoveBy({ -1, 0 });
            poop2.Reframe({ 1, 0 });
            
            if (stinkLevel == 0 && poop1.height >= TANK_H / 3)
            {
                stinkLevel++;
                poop1.ChangeColor("", "POOP");
                poop2.ChangeColor("", "POOP");
            }
            else if (poop1.height >= TANK_H / 2)
            {
                poop1.ChangeSeq(wormDance ? "~ " : " ~");
                poop2.ChangeSeq(wormDance ? "~ " : " ~");
                wormDance = !wormDance;
            }

            if (poop1.height >= TANK_H - 2)
            {
                stinkLevel++;

                boulder.ChangeColor("", "OCHRE");
                shooter.ChangeColor("POOP");
            }

            counter = 1;
        }

        
        Pause(10 MS);
    }

    Pause(1 M);
}


void MakeTank(const Coord& where)
{
    Scr::AtCoord(where);
    Scr::SetColor(TANK_COLOR);


    CanvasDraw(Fmt("' ' '_'_%d ' '", TANK_W - 2));

    for (int i = 2; i < TANK_H; i++) {
        CanvasDraw(Fmt("d l%d '|' ' '_%d '|'", TANK_W, TANK_W - 2));
    }

    CanvasDraw(Fmt("d l%d '*' '-'_%d '*'", TANK_W, TANK_W - 2));
}


int RandInt(int st, int end)
{
    return st + std::rand() % (end - st + 1);
}


int RandInt(int pool[], int length)
{
    return pool[std::rand() % length];
}