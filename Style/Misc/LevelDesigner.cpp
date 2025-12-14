#include "../Modules/style.h"
#include <fstream>
#include <deque>


const int height = Scr::HEIGHT;
const int width = Scr::WIDTH - 25;
const std::string heptStr = "._.\n|||\n|||\n|||\n.-.";

const std::string sideBarFg = "CYAN";
const std::string sideBarBg = "BLACK";
const std::string selFg = "LIGHTYEL";
const std::string selBg = "VIOLET";


//undoing/redoing
std::deque<std::pair<Coord, Pixel>> history;
int present = -1;  //points to last element rather than after it!!

void AddHistory(std::pair<Coord, Pixel> team)
{
    for (int i = present; i >= 0; i--)
    {
        auto pair = history[i];
        if (pair.first == team.first && pair.second == team.second)
            return;
    }

    if (team.second == Pixel())
        return;

    if (present == history.size() - 1)
        history.push_back(team);
    else
        history[present+1] = team;

    if (history.size() > 200)
        history.pop_front();
    else
        present++;
}


int main()
{
    PixelGrid myGrid(height, width);
    myGrid.Render();


    Grid sideBar({ 1, width + 3 }, "", "BLUE", ' ', ' ', ' ');
    sideBar.Configure(7, 1, 6, 20, "both", sideBarBg, sideBarFg);

    sideBar.Update({1, 1}, "FG: WHITE\nBG: BLACK");
    sideBar.Update({2, 1}, "HISTORY");
    sideBar.Update({3, 1}, "PIXEL ID\n<NULL>");
    sideBar.Update({4, 1}, "PENALTY\nHORZ: 20\nVERT: 20");
    sideBar.Update({5, 1}, "STICKY\nOFF");
    sideBar.Update({6, 1}, "FOCUS CHAR\n\n'_'");
    sideBar.Update({7, 1}, heptStr, "PINK", "ROSE");
    sideBar.Draw();

    Button sideBtn(&sideBar, State(Coord(), selFg, selBg, "I"), 1, 45, true);


    // START!
    int horzPenalty = 20, vertPenalty = 20, waitCounter = 0;

    Pixel now = myGrid.GetPixel({1, 1});
    char prevChar = now.c;
    std::string prevFg = now.fg, prevBg = now.bg;


    Coord cursCoord = { 1, 1 };
    std::string fg = "WHITE", bg = "BLACK";


    char focusChar = '_';
    Scr::AtCoord();
    Scr::SetColor("", bg);
    
    outBuff << focusChar;
    Scr::MoveCursor(Dir::LEFT);
    outBuff.flush();


    //id-mappings
    std::map<byte, std::string> idMap =  { { 0, "NULL" } };
    int prevId = 0;  //to prevent repetition


    //second menubar
    // Grid devMenu({ 1, width + 3 }, "", "COTCANDY", ' ', ' ', ' ');
    // devMenu.Configure(7, 1, 6, 20, "both", "LOVE");
    // devMenu.Draw();


    //flags and stuff
    bool atSideFrame = false;
    bool sticky = false;
    bool shiftOn = false;
    int frameReturn = 0;
    int nextId = 1;
    int idNow = 0;


    while (true)
    {
        int key;

        if (waitCounter > 0)  waitCounter--;

        else if (atSideFrame)
        {
            Coord coord = sideBtn.Check();

            if (coord == Button::BACKSPACE)
            {
                atSideFrame = false;
                sideBtn.NoHighlight(true);
                frameReturn = 5;
            }

            else if (coord == Button::ESCAPE)
            {
                break;
            }

            else if (coord == Button::RIGHT_CLICK && sideBtn.selCoord.ROW == 7)
            {
                // devMenu.Draw();
                // MicroSleep(300 MS);

                // while (!KeyDown(VK_RBUTTON))
                //     MicroSleep(50 MS);
                
                // sideBar.Draw();
                
                waitCounter = 500;
            }

            else if (coord.isValid())
            {
                switch (coord.ROW)
                {
                    case 1:
                    {
                        bool isBg = (Entry::TakeInput(sideBar, {1, 1}, "1. FG\n2. BG", "", "", "char") == "2");
                        std::string color;

                        MicroSleep(75 MS);

                        while (true)
                        {
                            color = Entry::TakeInput(sideBar, {1, 1}, "ENTER", selFg, selBg, "alpha");
                            if (!colors.count(color))
                            {
                                sideBar.Update({1, 1}, "FAULTY COLOR", "LIGHTYEL", "LOVE");
                                sideBar.Render();
                                Pause(850 MS);
                            }
                            else
                            {
                                sideBar.Update({1, 1}, "FG: " + (isBg ? fg : color) + "\nBG: " + (isBg ? color : bg));
                                sideBar.Render();
                                break;
                            }
                        }

                        if (isBg) bg = color;
                        else  fg = color;

                        waitCounter = 100;
                        break;
                    }

                    case 2:
                    {
                        MicroSleep(200 MS);
                        
                        sideBar.Update({2, 1}, "", "SKY", "PRUSSIAN");
                        sideBar.Render();

                        while ((key = GetKey()) != VK_RETURN)
                        {
                            if (key == VK_LEFT && present > 0)
                            {
                                auto thisElm = history[present--];

                                bool found = false;
                                for (int i = present; i >= 0; i--)
                                {
                                    auto pair = history[i];
                                    
                                    if (pair.first == thisElm.first)
                                    {
                                        found = true;
                                        Scr::AtCoord(pair.first);
                                        pair.second.Print();
                                        myGrid.GetPixel(pair.first) = pair.second;
                                        break;
                                    }
                                }

                                if (!found)
                                {
                                    Scr::AtCoord(thisElm.first);
                                    Scr::SetColor("WHITE", "BLACK");
                                    outBuff << ' ';
                                    myGrid.GetPixel(thisElm.first) = Pixel();
                                }
                            }
                            else if (key == VK_RIGHT && present < history.size() - 1)
                            {
                                auto pair = history[++present];

                                Scr::AtCoord(pair.first);
                                pair.second.Print();
                            }

                            outBuff.flush();
                            MicroSleep(60 MS);
                        }


                        waitCounter = 100;
                        break;
                    }

                    case 3:
                    {
                        if (idNow)
                        {
                            sideBar.Update({3, 1}, "PIXEL ID\n...");
                            idNow = 0;
                        }
                        else
                        {
                            sideBar.Update({3, 1}, "");
                            sideBar.Render();
                            MicroSleep(100 MS);

                            std::string idName = Entry::TakeInput(sideBar, {3, 1}, "ENTER ID", "", "", "all");

                            for (auto pair : idMap)
                            {
                                if (pair.second == idName)
                                {
                                    idNow = pair.first;
                                    break;
                                }
                            }

                            if (idNow == 0)
                            {
                                idNow = nextId++;
                                idMap[idNow] = idName;

                                std::string buff;
                                for (auto pair : idMap)
                                    buff += std::to_string(pair.first) + " : " + pair.second + "\n";
                            }

                            sideBar.Update({3, 1}, "NOW ASSIGNING\n<" + idName + std::string(1, '>'));
                        }

                        waitCounter = 300;
                        break;
                    }
                    
                    case 4:
                    {
                        MicroSleep(50 MS);

                        bool vert = false;;
                        if (Entry::TakeInput(sideBar, {4, 1}, "H | V", "COTCANDY", "", "char") == "V")
                            vert = true;

                        Pause(50 MS);

                        int n = std::stoi(Entry::TakeInput(sideBar, {4, 1}, "ENTER", "", "", "digit"));

                        if (vert)
                            vertPenalty = n;
                        else
                            horzPenalty = n;

                        sideBar.Update({4, 1}, "PENALTY\nHORZ: " + std::to_string(horzPenalty) + "\nVERT: " + std::to_string(vertPenalty), selFg, selBg);
                        waitCounter = 300;
                        break;
                    }

                    case 5:
                    {
                        sticky = not sticky;
                        sideBar.Update({5, 1}, "STICKY\n" + std::string(sticky ? "ON" : "OFF"));
                        waitCounter = 200;
                        break;
                    }

                    case 6:
                    {
                        MicroSleep(100 MS);

                        std::string keyChar = Entry::TakeInput(sideBar, {6, 1}, "ENTER", "", "", "char");
                        sideBar.Update({6, 1}, "FOCUS CHAR\n\n'" + keyChar + "'");
                        focusChar = keyChar[0];
                        
                        waitCounter = 50;
                    }
                }
            }

            sideBar.Render();
        }
        
        else if (!atSideFrame && (key = GetKey()))
        {
            if (frameReturn)
            {
                sideBar.Draw();
                frameReturn--;
            }
            
            bool moved = VK_LEFT <= key && key <= VK_DOWN;

            if ((moved || key == VK_TAB) && !sticky)
            {
                Scr::SetColor(prevFg, prevBg);
                outBuff << prevChar;
                Scr::MoveCursor(Dir::LEFT);
            }
            else if (sticky && focusChar == ' ')
            {
                outBuff << ' ';
                Scr::MoveCursor(Dir::LEFT);
            }


            switch (key)
            {
                case VK_LEFT:
                    if (cursCoord.COL > 1) cursCoord.COL--;
                    break;

                case VK_RIGHT:
                    if (cursCoord.COL < width) cursCoord.COL++;
                    break;
                    
                case VK_DOWN:
                    if (cursCoord.ROW < height) cursCoord.ROW++;
                    break;

                case VK_UP:
                    if (cursCoord.ROW > 1) cursCoord.ROW--;
                    break;

                case VK_TAB:
                    atSideFrame = true;
                    sideBtn.NoHighlight(false);
                    sideBar.Render();
                    break;

                case VK_RETURN:
                    if (idNow)
                    {
                        Pixel pixel = myGrid.GetPixel(cursCoord);
                        pixel.id = idNow;
                    }
                    break;

                case VK_SHIFT:
                    shiftOn = true;
                    waitCounter = 100;
                    continue;

                default:
                {
                    char c = MapVirtualKey(key, MAPVK_VK_TO_CHAR);
                    if (shiftOn && shifted.count(c))
                        c = shifted[c];

                    if (32 <= c && c <= 126)
                    {
                        Pixel& pixel = myGrid.GetPixel(cursCoord);
                        if (pixel.c != c || strcmp(pixel.fg, fg.c_str()) || strcmp(pixel.bg, bg.c_str()) || idNow && pixel.id != idNow)
                        {
                            AddHistory({cursCoord, pixel});

                            pixel.c = c;
                            strcpy(pixel.bg, bg.c_str());
                            strcpy(pixel.fg, fg.c_str());
                            if (idNow) pixel.id = idNow;

                            AddHistory({ cursCoord, pixel });

                            prevChar = pixel.c;
                            prevBg = bg;
                            prevFg = fg;

                            Scr::SetColor(pixel.fg, pixel.bg);
                            outBuff << pixel.c;
                            Scr::MoveCursor(Dir::LEFT);
                        }
                    }
                }
            }


            if (moved)
            {
                Pixel& pixel = myGrid.GetPixel(cursCoord);

                if (!sticky)
                {
                    prevChar = pixel.c;
                    prevBg = pixel.bg;
                    prevFg = pixel.fg;

                    if (pixel.id != prevId)
                    {
                        prevId = pixel.id;
                        sideBar.Update({3, 1}, "PIXEL ID\n<" + idMap[pixel.id] + '>');
                        sideBar.Render();
                    }
                }
                else
                {
                    pixel.c = focusChar;

                    strcpy(pixel.fg, fg.c_str());
                    strcpy(pixel.bg, bg.c_str());

                    if (idNow)
                        pixel.id = idNow;

                    AddHistory({cursCoord, pixel});
                }


                Scr::AtCoord(cursCoord);
                Scr::SetColor(fg, bg);
                
                outBuff << (focusChar == ' ' ? '_' : focusChar);
                Scr::MoveCursor(Dir::LEFT);
            }

            waitCounter = (key == VK_UP || key == VK_DOWN) ? vertPenalty : (key == VK_LEFT || key == VK_RIGHT) ? horzPenalty : 30;
            shiftOn = false;
        }

        Pause(1 MS);
    }


    std::ofstream outFile("drawings.log");
    
    for (auto pair : idMap)
    {
        outFile << "PixelMap " << (pair.first == 0 ? "theme" : pair.second) << "( {";

        for (int r = 1; r <= height; r++)
        {
            for (int c = 1; c <= width; c++)
            {
                Pixel pixel = myGrid.GetPixel({r, c});
                
                if (pixel.id == pair.first)
                {
                    if (!pair.first && pixel == Pixel())
                        continue;

                    outFile << "\n    { Coord(" << r << ", " << c << "), Pixel('" << pixel.c << "', \"" << pixel.fg << "\", \"" << pixel.bg << "\") },";
                }
            }
        }

        outFile << "\n} );\n\n";
    }

    outFile.close();
    Pause();
}
