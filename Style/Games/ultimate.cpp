#include "../Modules/style.h"


class Human
{
    Coord bottomCenter;
    int height, health = 100, level = 1;
    std::vector<Figure*> parts;

    const char MORPH_SEQ[4] = { '/', '|', '\\', '-' };
    int morphSeqNow = 0;


    public:
        enum class Mode
        {
            SPREAD,
            TOGETHER,
            MORPHBALL
        } seq;

        Group* body;
        std::string playerName;


        Human(Coord bottomCenter, int height = 5, std::string fgColor = "WHITE", std::string bgColor = Scr::SCREEN_BG, Mode form = Mode::SPREAD)
            : seq(form), bottomCenter(bottomCenter)
        {
            parts = {
                new Point('O', bottomCenter - Coord(height - 1, 0)),   // 0  =>  head
                new Point('/', bottomCenter - Coord(height - 3, 1)),   // 1  =>  left hand
                new Point('\\', bottomCenter - Coord(height - 3, -1)), // 2  =>  right hand
                new Point('/', bottomCenter - Coord(0, 1)),            // 3  =>  left leg  (also morphball left '@')
                new Point('\\', bottomCenter - Coord(0, -1)),          // 4  =>  right leg (also morphball right '@')
                new Point(' ', bottomCenter),                          // 5  =>  anus      (also morphball center)
                new Point(' ', bottomCenter - Coord(0, 2)),            // 6  =>  (morphball left bound)
                new Point(' ', bottomCenter + Coord(0, 2))             // 7  =>  (morphball right end)
            };

            for (int h = height - 2; h > 0; h--)
                parts.push_back(new Point('|', bottomCenter - Coord(h, 0)));

            body = new Group(parts);
            body->ChangeColor(fgColor, bgColor);

            if (form != Mode::SPREAD) TransformSeq(form, 0);
        }


        void Move(Coord towards, bool toggleSeq = true)
        {
            for (int i = parts.size() - 1; i >= 0; i--)
            {
                if (seq == Mode::MORPHBALL)
                {
                    if (i >= 3 && i <= 7) parts[i]->MoveBy(towards);
                    else parts[i]->thisState.coord += towards;
                }
                else
                {
                    if (i == 6 || i == 7) parts[i]->thisState.coord += towards;
                    else parts[i]->MoveBy(towards);
                }
            }

            if (toggleSeq) ToggleSeq();
            bottomCenter += towards;
        }


        void Draw()
        {
            body->ReDraw();
        }


        void ToggleSeq(bool ballRollLeft = false)
        {
            seq = (seq == Mode::SPREAD) ? Mode::TOGETHER : (seq == Mode::TOGETHER) ? Mode::SPREAD : Mode::MORPHBALL;

            if (seq == Mode::SPREAD)
            {
                parts[1]->ChangeSeq("/");
                parts[2]->ChangeSeq("\\");
                parts[3]->ChangeSeq("/");
                parts[4]->ChangeSeq("\\");
                parts[5]->ChangeSeq(" ");
            }
            else if (seq == Mode::TOGETHER)
            {
                for (int i = 1; i <= 4; i++)
                    parts[i]->ChangeSeq(" ");

                parts[5]->ChangeSeq("|");
            }
            else
            {
                morphSeqNow = (morphSeqNow + (1 - 2 * ballRollLeft)) % 4;
                parts[5]->ChangeSeq(std::string(1, MORPH_SEQ[morphSeqNow]));
            }
        }


        void TransformSeq(Mode form, int transition = 150 MS)
        {
            if (form == Mode::MORPHBALL)
            {
                for (int i = 0; i < parts.size(); i++)
                {
                    if (i >= 3 && i <= 7) continue;
                    Pause(transition);
                    parts[i]->ChangeSeq(" ");
                }

                Pause(transition);

                parts[3]->ChangeSeq("@");
                parts[4]->ChangeSeq("@");
                Pause(transition);
                parts[6]->ChangeSeq("(");
                parts[7]->ChangeSeq(")");
                Pause(transition);
                parts[5]->ChangeSeq("/");
                Pause(2 * transition);

                morphSeqNow = 0;
                seq = Mode::MORPHBALL;
            }
            
            else if (seq == Mode::MORPHBALL)
            {
                parts[5]->ChangeSeq(" ");
                Pause(2 * transition);
                parts[6]->ChangeSeq(" ");
                parts[7]->ChangeSeq(" ");
                Pause(transition);
                parts[3]->ChangeSeq(" ");
                parts[4]->ChangeSeq(" ");
                Pause(transition);

                Pause(transition);

                parts[3]->ChangeSeq("/");
                Pause(transition);
                parts[4]->ChangeSeq("\\");
                Pause(transition);
                parts[1]->ChangeSeq("/");
                Pause(transition);
                parts[2]->ChangeSeq("\\");
                Pause(transition);
                parts[0]->ChangeSeq("O");
                Pause(transition);

                for (int i = 8; i < parts.size(); i++)
                {
                    parts[i]->ChangeSeq("|");
                    Pause(transition);
                }

                seq = Mode::SPREAD;
                if (form == Mode::TOGETHER) ToggleSeq();
            }

        }


        Coord GetBottom()
        {
            return bottomCenter;
        }


        // Humans aren't meant to be invisible...
        void ChangeColor(std::string fgColor, std::string bgColor = "")
        {
            for (int i = 0; i < parts.size(); i++)
            {
                if (fgColor != "")
                    parts[i]->thisState.fgColor = fgColor;
                if (bgColor != "")
                    parts[i]->thisState.bgColor = bgColor;

                if (seq == Mode::MORPHBALL && i >= 3 && i <= 7
                    || seq != Mode::MORPHBALL && !(i >= 3 && i <= 7))
                    parts[i]->Draw();
            }
        }
};


class GameDisplay
{
    enum Levels { LAVAH, MARIDIA, BRINSTAR, CRATERIA };
    
    // label, div, font, background ambience
    std::map<Levels, std::vector<std::string>> levelColors = {
        { LAVAH, { "RED", "%50;0;0", "BLUE", "%20;0;0" } },
        { MARIDIA, { "BLUE", "VIOLET", "RED", "%0;0;20" } },
        { BRINSTAR, { "PARROT", "%0;50;0", "BROWN", "%0;20;0" } },
        { CRATERIA, { "GRAY", "%50;50;50", "WHITE", "%20;20;20" } }
    };


    Coord topLeft;     // top left corner of available space, not frame
    int width, height; // again, this is for available space and not for the frame


    public:
        bool active = true;

        // offset is one less than the dimensions
        // frameType  =>  "none", "regular", "hollow-YELLOW"
        GameDisplay(Coord topLeftCorner = { 1, 1 }, int totalWidth = Scr::WIDTH, int totalHeight = Scr::HEIGHT, std::string frameType = "regular")
            : topLeft(topLeftCorner + Coord(1, 1)), width(totalWidth - 2), height(totalHeight - 2)
        {
            // if frame is disabled, its space is filled with the default BG
            Scr::Paint("BLACK", topLeft, topLeft + Coord(height - 1, width - 1));

            if (frameType != "none")
            {
                Scr::SetColor("", "BLACK");
                Figure::Frame(topLeft - Coord(1, 1), topLeft + Coord(height, width), (frameType == "regular") ? "" : frameType.substr(7));
            }
        }


        void LoadIntro()
        {
            Scr::Paint("%20;0;0", topLeft, topLeft + Coord(height - 1, width - 1), -1, true);


            // welcome label
            int horzPad = 15;
            Coord introCoord = topLeft + Coord(1, (width - (14 + 2 * horzPad)) / 2);
            Grid* introLabel = Grid::MakeLabel("W E L C O M E!", { Coord(), "WHITE", "BLACK" }, introCoord, horzPad, 1);

            // intro transition
            for (int wb = 0, bw = 230; wb <= 230; wb++, bw--)
            {
                introLabel->Update({1, 1}, "", Fmt("%%d;%d;%d", bw, bw, bw), Fmt("%%d;%d;%d", wb, wb, wb));
                introLabel->Render();
                Pause(8 MS);
            }


            // div grid
            const int divH = 25;
            const int divW = 58;
            const Coord divCent = topLeft + Coord(height / 2, width / 2);
            const Coord divTL = divCent - Coord(divH / 2, divW / 2);
            Scr::SCREEN_BG = "%60;0;0";
            Block div(" ", divW, divH, divTL, Scr::SCREEN_BG);


            // the walker
            int horzDist = 6, vertDist = 3;
            Human walker(divTL - Coord(vertDist, horzDist), 5, "%20;0;0", "%20;0;0");
            walker.TransformSeq(Human::Mode::MORPHBALL);
            walker.body->dir = { 0, 1 };
            int wb = 0, incr = 2;


            // intro man
            Human introMan(divCent - Coord(3, 1), 5, "RED");
            introMan.TransformSeq(Human::Mode::MORPHBALL);


            // entry grid
            const int inputW = 30;
            const int inputH = 5;
            Grid input(divCent + Coord(4, -inputW / 2));
            input.Configure(1, 1, inputH, inputW, "none", "BLACK", "PARROT");
            

            // label grid
            Grid tag(divCent - Coord(0, inputW / 2));
            tag.Configure(1, 1, 3, inputW, "none");
            tag.Update({1, 1}, "LAVAH", "RED");
            tag.Draw();


            // initial name entry
            std::string playerName = Entry::TakeInput(input, {1, 1}, "ENTER YOUR NAME");


            // start button grid
            Grid start(divCent - Coord(9, inputW / 2));
            start.Configure(1, 1, 3, inputW, "none", "RED", "YELLOW");
            start.Update({1, 1}, "START!");
            Button btn(&start, { }, 5, 30, true);

            Pause(300 MS);

            int keyTicks = 0;
            int counter = 200;
            Levels level = LAVAH;  // norfair, maridia, brinstar, crateria (in no order of difficulty)

            while (true)
            {
                // div box morphball animation
                if (counter % 100 == 0) introMan.ToggleSeq();


                // walker movement
                if (counter % 30 == 0)
                {
                    std::string prevBgStore = Scr::SCREEN_BG;
                    Scr::SCREEN_BG = "%20;0;0";

                    Coord& dir = walker.body->dir;
                    walker.Move(dir);

                    Coord delta = walker.GetBottom() - divTL;
                    if (delta == Coord(-vertDist, -horzDist - 1)) dir = { 0, 1 };
                    else if (delta == Coord(-vertDist, divW + horzDist)) dir = { 1, 0 };
                    else if (delta == Coord(divH + vertDist + 3, divW + horzDist)) dir = { 0, -1 };
                    else if (delta == Coord(divH + vertDist + 3, -horzDist - 1)) dir = { -1, 0 };

                    walker.ChangeColor(Fmt("%%d;%d;%d", wb, wb, wb));
                    wb += incr;
                    if (wb == 100 || wb == 0) incr = -incr;

                    Scr::SCREEN_BG = prevBgStore;
                }


                // keypress checks
                Coord coord = btn.Check();
                // level invoker
                if (coord == Coord(1, 1))
                {
                    if (level == LAVAH) StartLavah();
                    return;
                }
                else if (coord == Button::ESCAPE)
                {
                    State state = start.GetAccess({1, 1})->style;
                    std::string fg = state.fgColor, bg = state.bgColor;

                    start.Update({1, 1}, "", "BLUE", "VIOLET");
                    start.Render();

                    playerName = Entry::TakeInput(input, {1, 1}, "RE-ENTER");

                    start.Update({1, 1}, "", fg, bg);
                    start.Render();
                    Pause(300 MS);
                }


                // right click level selector
                if (counter % 200 == 0 && KeyDown(VK_RBUTTON))
                {
                    std::string btnColor, divColor, fontColor, labelName;
                    level = (Levels)((level + 1) % 4);

                    auto colorList = levelColors[level];
                    btnColor = colorList[0];
                    divColor = colorList[1];
                    fontColor = colorList[2];

                    labelName = (level == LAVAH) ? "LAVAH" : (level == MARIDIA) ? "MARIDIA" : (level == BRINSTAR) ? "BRINSTAR" : "CRATERIA";

                    tag.Update({1, 1}, labelName, btnColor, divColor);
                    start.Update({1, 1}, "", fontColor, btnColor);

                    div.ChangeColor("", divColor);
                    introMan.body->ChangeColor(btnColor, divColor);
                    introMan.Draw();

                    tag.Render();
                    start.Render();
                    input.Render();
                }

                if (--counter == 0) counter = 200;
                Pause(1 MS);
            }
        }

        // void StartLavah()
        // {
        //     Coord blockTL = { Scr::HEIGHT / 2, Scr::WIDTH / 2 };
        //     int blockH = 5, blockW = 10;
        //     const std::string blockColor = "%100;100;100";
        //     Block block(" ", blockW, blockH, blockTL, blockColor);
            
        //     outBuff.bufferSize = 100;


        //     const int roadDefaultBg[] = { 0, 0, 50 };
        //     const int gradientStart[] = { 0, 0, 0 };

        //     std::string roadDefaultBgStr = Fmt("%%d;%d;%d", roadDefaultBg[0], roadDefaultBg[1], roadDefaultBg[2]);
        //     std::string skyBg = "BLACK";

        //     Scr::Paint(skyBg);


        //     auto colorRoadPixel = [&]()
        //     {
        //         // find the start coord and decide bar dimension
        //         const int maxBarW = 5;
        //         const int startRow = height / 2;

        //         Coord roadCoord = {
        //             Ranint(startRow, height - 1),
        //             Ranint(1, width - 2)
        //         };


        //         // if not within the triangle, return
        //         const int startCol = width / 3;
        //         const int startWidth = width - 2 * startCol;

        //         double percH = (double)(roadCoord.ROW - startRow) / (height - startRow);
        //         int currCol = 1 + startCol * (1 - percH);
        //         int currWidth = startWidth + (width - startWidth - 1) * percH;

        //         int endCol = currCol + currWidth;

        //         if (roadCoord.COL < currCol || roadCoord.COL > endCol)
        //             return;


        //         int barW = std::min(maxBarW, Scr::WIDTH - roadCoord.COL);

                
        //         // compute the color gradient
        //         int deltaR = gradientStart[0] - roadDefaultBg[0];
        //         int deltaG = gradientStart[1] - roadDefaultBg[1];
        //         int deltaB = gradientStart[2] - roadDefaultBg[2];


        //         const int gradientColor[] = {
        //             roadDefaultBg[0] + deltaR * percH,
        //             roadDefaultBg[1] + deltaG * percH,
        //             roadDefaultBg[2] + deltaB * percH
        //         };

        //         std::string gradientColorStr = Fmt("%%d;%d;%d", gradientColor[0], gradientColor[1], gradientColor[2]);


        //         // check overlaps and set states
        //         int len = barW;
        //         std::string currRoadColor = Ranint(0, 1) ? roadDefaultBgStr : gradientColorStr;
        //         std::string currColor = currRoadColor;

        //         if (roadCoord.ROW >= blockTL.ROW && roadCoord.ROW < blockTL.ROW + blockH)
        //         {
        //             if (roadCoord.COL >= blockTL.COL && roadCoord.COL < blockTL.COL + blockW)
        //             {
        //                 len = 1;
        //                 currColor = blockColor;
        //             }
        //             else if (roadCoord.COL < blockTL.COL && barW > blockTL.COL - roadCoord.COL)
        //                 len = blockTL.COL - roadCoord.COL;
        //         }
                

        //         // color pixels
        //         Scr::AtCoord(roadCoord);
        //         Scr::SetColor("", currColor);

        //         outBuff << std::string(len, ' ');
        //     };

        //     for (int i = 0; true; i++)
        //     {
        //         Coord diff = { 0, 0 };

        //         if (i % 50 == 0) {
        //             if (KeyDown(VK_UP)) diff.ROW = -1;
        //             else if (KeyDown(VK_DOWN)) diff.ROW = 1;
        //         }
        //         if (i % 10 == 0) {
        //             if (KeyDown(VK_RIGHT)) diff.COL = 1;
        //             else if (KeyDown(VK_LEFT)) diff.COL = -1;
        //         }
                
        //         if (diff != Coord(0, 0)) {
        //             blockTL += diff;
        //             block.MoveTo(blockTL);
        //         }

        //         colorRoadPixel();
        //     }
        // }
};


int main()
{
    GameDisplay game1({1, 1}, Scr::WIDTH, Scr::HEIGHT, "hollow %50;0;0");
    game1.LoadIntro();
    Pause();
}