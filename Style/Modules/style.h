#include <windows.h>
#include <cstdio>
#include <map>
#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <sstream>
#include <ctime>

#define MS *1000
#define S  *1000000
#define M  *60000000


class Coord
{
    public:
        int ROW, COL;

        Coord(int x = -1, int y = -1)
        {
            ROW = x;
            COL = y;
        }


        Coord operator+(const Coord& other) const
        {
            return Coord(ROW + other.ROW, COL + other.COL);
        }


        Coord operator+(int offset)
        {
            return Coord(ROW + offset, COL + offset);
        }


        Coord operator-(const Coord& other) const
        {
            return Coord(ROW - other.ROW, COL - other.COL);
        }


        void operator+=(const Coord& other)
        {
            ROW += other.ROW;
            COL += other.COL;
        }


        void operator+=(int offset)
        {
            ROW += offset;
            COL += offset;
        }


        void operator-=(const Coord& other)
        {
            ROW -= other.ROW;
            COL -= other.COL;
        }


        bool operator==(const Coord& other)
        {
            return ROW == other.ROW && COL == other.COL;
        }


        bool operator!=(const Coord& other)
        {
            return !(*this == other);
        }


        bool isValid() const
        {
            return ROW > 0 && COL > 0;
        }
};


class State
{
    public:
        Coord coord;
        std::string fgColor, bgColor, style;

        State(const Coord& coord = { -1, -1 }, const std::string& fgColor = "", const std::string& bgColor = "", const std::string& style = "NONE")
        {
            this->coord = coord;
            this->fgColor = fgColor;
            this->bgColor = bgColor;
            this->style = style;
        }


        State(const State& state)
        {
            coord = state.coord;
            fgColor = state.fgColor;
            bgColor = state.bgColor;
            style = state.style;
        }


        bool operator==(State other)
        {
            return coord == other.coord && fgColor == other.fgColor && bgColor == other.bgColor;
        }


        bool operator!=(State other)
        {
            return !(*this == other);
        }
};


extern State stateNow;


class OutBuffer
{
    private:
        std::string buffer;
        bool line_start = true;


    public:
        int lpad = 0, bufferSize;
        bool modifyStateNow = true;
        
        OutBuffer(int bufferSize)
        {
            this->bufferSize = bufferSize;
            std::cout << "\033[38;2;255;255;255m\033[?25l" << std::flush;
            srand(time(0));
        }


        bool IsLineStart(bool is_it)
        {
            line_start = is_it;
        }


        OutBuffer& operator<<(const std::string& other)
        {
            if (lpad)
            {
                std::string padding = std::string(lpad, ' ');

                if (line_start)
                {
                    buffer += padding;
                    line_start = false;
                    if (modifyStateNow)
                        stateNow.coord.COL += lpad;
                }

                if (other.find('\n') != std::string::npos)
                {
                    for (int i = 0; i < other.length(); i++)
                    {
                        buffer += other[i];
                        if (modifyStateNow)
                            stateNow.coord.COL++;

                        if (other[i] == '\n')
                        {
                            buffer += padding;
                            if (modifyStateNow)
                            {
                                stateNow.coord.ROW++;
                                stateNow.coord.COL = lpad + 1;
                            }
                        }
                    }
                }
                else
                {
                    buffer += other;
                    if (modifyStateNow)
                        stateNow.coord.COL += other.length();
                }
            }
            else
            {
                buffer += other;

                int newlCount = std::count(other.begin(), other.end(), '\n');

                if (modifyStateNow)
                {
                    if (newlCount == 0)
                    {
                        stateNow.coord.COL += other.length();
                    }
                    else
                    {
                        stateNow.coord.ROW += newlCount;
                        stateNow.coord.COL += other.length() - other.rfind('\n') - 1;
                    }
                }
            }

            if (buffer.length() > bufferSize) flush();
            
            return *this;
        }


        OutBuffer& operator<<(int other)
        {
            return *this << std::to_string(other);
        }


        OutBuffer& operator<<(char other)
        {
            if (other == '\0') {
                return *this;
            }

            return *this << std::string(1, other);
        }


        void flush()
        {
            return;
            std::cout << buffer << std::flush;
            buffer = "";
        }


        ~OutBuffer()
        {
            *this << "\033[0m";
            flush();
        }
};


namespace Dir
{
    enum Direction {
        NONE = 0,
        UP    = 1,
        DOWN  = 2,
        RIGHT = 4,
        LEFT  = 8
    };
}


int CanvasDraw(const std::string& cmd, bool flush = true, bool is_line_start = false);
std::string Fmt(const char *cmd, ...);
std::string Center(std::string string, int width, char padChar = ' ');
void MicroSleep(long long microseconds);
Coord ParseCoord(const Coord& coord);
std::string ColorSweep(int delayMs = -1);
void Pause(int x = 1 M);
bool KeyDown(int key);
int GetKey();
int Ranint(int st, int end);


extern std::map<std::string, char> cursorCtrls;
extern std::map<std::string, std::string> colors;
extern std::map<std::string, std::string> styles;
extern std::map<char, char> shifted;
extern OutBuffer outBuff;
extern int wait;


class Scr
{
    public:
        static std::vector<State> LIFOSaves;
        static std::map<std::string, State> mapSaves;
    
        static const int WIDTH = 188;
        static const int HEIGHT = 60;
        
        static std::string SCREEN_BG;

    
        static void SetColor(const std::string& fgColor, const std::string& bgColor = "")
        {
            outBuff.modifyStateNow = false;

            if (fgColor != "" && fgColor != stateNow.fgColor)
            {
                if (fgColor[0] == '%')
                    outBuff << "\033[38;2;" << fgColor.substr(1) << "m";
                else
                    outBuff << "\033[38;2;" << colors[fgColor] << "m";

                stateNow.fgColor = fgColor;
            }
            
            if (bgColor != "" && bgColor != stateNow.bgColor)
            {
                if (bgColor[0] == '%')
                    outBuff << "\033[48;2;" << bgColor.substr(1) << "m";
                else
                    outBuff << "\033[48;2;" << colors[bgColor] << "m";

                stateNow.bgColor = bgColor;
            }

            outBuff.modifyStateNow = true;
        }


        static void SetStyle(const std::string& style)
        {
            outBuff.modifyStateNow = false;

            if (style == "NONE")
            {
                outBuff << "\033[22m\033[23m\033[24m\033[29m";
                stateNow.style = "NONE";
            }
            else if (style != "")
            {
                outBuff << "\033[" << styles[style];

                if (style != "H" && style != "UH")
                    stateNow.style = style;
            }

            outBuff.modifyStateNow = true;
        }


        static void MoveCursor(int where, int n = 1)
        {
            outBuff.modifyStateNow = false;

            outBuff << "\033[" << n;

            switch (where)
            {
                case Dir::UP:
                    outBuff << cursorCtrls["UP"];
                    stateNow.coord.ROW -= n;
                    break;
                
                case Dir::DOWN:
                    outBuff << cursorCtrls["DOWN"];
                    stateNow.coord.ROW += n;
                    break;
                
                case Dir::RIGHT:
                    outBuff << cursorCtrls["RIGHT"];
                    stateNow.coord.COL += n;
                    break;
                
                case Dir::LEFT:
                    outBuff << cursorCtrls["LEFT"];
                    stateNow.coord.COL -= n;
            }

            outBuff.modifyStateNow = true;
        }


        static Coord GetCurs(bool actual = false)
        {
            if (!actual)
                return stateNow.coord;

            HANDLE hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
            CONSOLE_SCREEN_BUFFER_INFO csbi = { };
            GetConsoleScreenBufferInfo(hConsoleOutput, &csbi);
            COORD res = csbi.dwCursorPosition;

            return Coord(res.X + 1, res.Y + 1);
        }


        static void AtCoord(const Coord& dest = { 1, 1 })
        {
            outBuff.modifyStateNow = false;

            Coord coord = ParseCoord(dest);
            int row = coord.ROW, col = coord.COL;
            
            outBuff << "\033[" << row << ";" << col << "H";
            stateNow.coord = dest;

            outBuff.modifyStateNow = true;
        }


        static void Paint(const std::string& color, Coord st = { 1, 1 }, Coord end = { -1, -1 }, int width = -1, bool setAsBg = false)
        {
            Scr::SaveState();

            if (setAsBg || st == Coord(1, 1) && end == Coord(-1, -1))
                Scr::SCREEN_BG = color;

            st  = ParseCoord(st) ;
            end = ParseCoord(end);

            if (width == -1)
                width = end.COL - st.COL + 1;

            Scr::AtCoord(st);
            Scr::SetColor("", color);
            
            for (int row = st.ROW; row < end.ROW; row++)
            {
                outBuff << std::string(width, ' ');
                Scr::MoveCursor(Dir::DOWN);
                Scr::MoveCursor(Dir::LEFT, width);
            }

            outBuff << std::string(width, ' ');
            Scr::RetrieveState(true, false);
            outBuff.flush();
        }


        static void Puts(const std::string& string, const Coord& point = { 0, 0 }, const std::string& align = "left", bool getCoord = true, bool getBg = true, bool getFg = false, bool getStyle = false)
        {
            bool dontOptimize = (getCoord || getBg || getFg);

            if (dontOptimize)
                Scr::SaveState();
            
            Scr::AtCoord(ParseCoord(point));


            int padding = 0;
            if (align == "center") {
                padding = (Scr::WIDTH -  string.length()) / 2;
            }
            else if (align == "right") {
                padding = Scr::WIDTH - string.length();
            }

            outBuff << std::string(padding, ' ') + string;

            if (dontOptimize)
                Scr::RetrieveState(getCoord, getBg, getFg);
        }


        static void SaveState()
        {
            LIFOSaves.push_back(stateNow);
        }


        static void SaveState(const std::string& tag, Coord coord = { -1, -1 }, const std::string& fgColor = "", const std::string& bgColor = "", const std::string& style = "")
        {
            mapSaves[tag] = State((coord.ROW == -1) ? GetCurs() : coord, fgColor, bgColor, style);
        }


        static void RetrieveState(bool getCoord = true, bool getBg = true, bool getFg = true, bool getStyle = false)
        {
            if (!LIFOSaves.empty())
            {
                State state = LIFOSaves.back();
                
                if (getCoord)
                    Scr::AtCoord(state.coord);
                if (getStyle)
                    Scr::SetStyle(state.style);

                if (getFg && getBg)
                    Scr::SetColor(state.fgColor, state.bgColor);
                else if (getBg)
                    Scr::SetColor("", state.bgColor);
                else if (getFg)
                    Scr::SetColor(state.fgColor, "");
            }
        }


        static void RetrieveState(const std::string& tag, bool getCoord = true, bool getBg = true, bool getFg = true, bool getStyle = false)
        {
            if (mapSaves.count(tag))
            {
                State state = mapSaves[tag];
                
                if (getCoord)
                    Scr::AtCoord(state.coord);
                if (getStyle)
                    Scr::SetStyle(state.style);

                if (getFg && getBg)
                    Scr::SetColor(state.fgColor, state.bgColor);
                else if (getBg)
                    Scr::SetColor("", state.bgColor);
                else if (getFg)
                    Scr::SetColor(state.fgColor, "");
            }
        }


        static void UpdateState(const State& state)
        {
            if (state.coord.isValid())
                Scr::AtCoord(state.coord);

            if (state.fgColor != "")
                Scr::SetColor(state.fgColor, state.bgColor);
            
            if (state.bgColor != "")
                Scr::SetStyle(state.style);
        }
};



class Figure
{
    public:
        State thisState;
        Coord dir;


        Figure(const Coord& vertex = { 0, 0 }, const std::string& fgColor = "", const std::string& bgColor = "")
        {
            thisState.coord = ParseCoord(vertex);
            thisState.fgColor = (fgColor == "") ? "WHITE" : fgColor;
            thisState.bgColor = (bgColor == "") ? Scr::SCREEN_BG : bgColor;
        }


        static void Fill(const Coord& corner1, const Coord& corner3, char character = '*')
        {
            std::string horz_string = std::string(corner3.COL - corner1.COL + 1, character);

            Scr::SaveState();
            Scr::AtCoord(corner1);

            for (Coord now = corner1; now.ROW <= corner3.ROW; now.ROW++)
            {
                Scr::AtCoord(now);
                outBuff << horz_string;
            }

            Scr::RetrieveState();
        }


        static void Frame(const Coord& _corner1, const Coord& _corner3, std::string hollowColor = "")
        {
            Coord corner1 = ParseCoord(_corner1);
            Coord corner3 = ParseCoord(_corner3);
            Coord corner2 = { corner1.ROW, corner3.COL };
            Coord corner4 = { corner3.ROW, corner1.COL };

            if (hollowColor == "")
            {
                Scr::Puts("+", corner1);
                Scr::Puts("+", corner2);
                Scr::Puts("+", corner3);
                Scr::Puts("+", corner4);

                Fill(corner1 + Coord(0, 1), corner2 - Coord(0, 1), '-');
                Fill(corner2 + Coord(1, 0), corner3 - Coord(1, 0), '|');
                Fill(corner4 + Coord(0, 1), corner3 - Coord(0, 1), '-');
                Fill(corner1 + Coord(1, 0), corner4 - Coord(1, 0), '|');
            }
            else
            {
                Scr::SetColor("", hollowColor);
                
                Fill(corner1, corner2, ' ');
                Fill(corner2, corner3, ' ');
                Fill(corner4, corner3, ' ');
                Fill(corner1, corner4, ' ');
            }
        }


        virtual void Clear(bool getCoord = true, bool getBg = false, bool getFg = false) { };


        virtual void Draw(bool getCoord = true, bool getBg = false, bool getFg = false) { };


        virtual void MoveTo(const Coord& where, bool getCoord = true, bool getBg = false, bool getFg = false) { };


        virtual void MoveBy(const Coord& diff, bool getCoord = true, bool getBg = false, bool getFg = false)
        {
            MoveTo(thisState.coord + diff, getCoord, getBg, getFg);
        }


        virtual Coord Next(bool reset = false) { }


        virtual void ChangeColor(const std::string& fgColor = "", const std::string& bgColor = "", bool optimize = false)
        {
            if (fgColor != "")
                thisState.fgColor = fgColor;
            if (bgColor != "")
                thisState.bgColor = bgColor;
            
            if (!optimize)
                Scr::SaveState();

            Draw();

            if (!optimize)
                Scr::RetrieveState();
        }


        virtual void ChangeSeq(const std::string& seq) { }


        virtual int Collides(const Coord& QPoint, const std::string& lineType, int dist = 1)  //lineType = horz, vert
        {
            int res = 0;
            bool notFound[] = { true, true, true, true };  //UP, DOWN, RIGHT, LEFT

            if (lineType == "horz" || lineType == "all")
            {
                Coord thisPt;

                while ( (thisPt = Next()).isValid() )
                {
                    int diff = thisPt.ROW - QPoint.ROW;

                    if (abs(diff) <= dist)
                    {
                        if (lineType == "all")
                        {
                            if (diff < 0 && notFound[0])
                            {
                                res += Dir::UP;
                                notFound[0] = false;
                            }
                            else if (diff > 0 && notFound[1])
                            {
                                res += Dir::DOWN;
                                notFound[1] = false;
                            }
                        }
                        else
                            return (diff > 0) ? Dir::DOWN : Dir::UP;
                    }
                }
            }
            
            if (lineType == "vert" || lineType == "all")
            {
                Coord thisPt;
                
                while ( (thisPt = Next()).isValid() )
                {
                    int diff = thisPt.COL - QPoint.COL;

                    if (abs(diff) <= dist)
                    {
                        if (lineType == "all")
                        {
                            if (diff > 0 && notFound[2])
                            {
                                res += Dir::RIGHT;
                                notFound[2] = false;
                            }
                            else if (diff < 0 && notFound[3])
                            {
                                res += Dir::LEFT;
                                notFound[3] = false;
                            }
                        }
                        else
                            return (diff > 0) ? Dir::RIGHT : Dir::LEFT;
                    }
                }
            }

            return res;
        }


        virtual int Collides(Figure& other, int dist = 1)
        {
            int res = 0;
            bool notFound[] = { true, true, true, true };  //UP, DOWN, RIGHT, LEFT
            Coord thisPt, otherPt;

            while ( (thisPt = this->Next()).isValid() )
            {
                while ( (otherPt = other.Next()).isValid() )
                {
                    if (thisPt.COL == otherPt.COL)
                    {
                        int diff = thisPt.ROW - otherPt.ROW;

                        if (abs(diff) <= dist)
                        {
                            if (diff < 0 && notFound[0])
                            {
                                res += Dir::UP;
                                notFound[0] = false;
                            }
                            else if (diff > 0 && notFound[1])
                            {
                                res += Dir::DOWN;
                                notFound[1] = false;
                            }
                        }
                    }

                    if (thisPt.ROW == otherPt.ROW)
                    {
                        int diff = thisPt.COL - otherPt.COL;

                        if (abs(diff) <= dist)
                        {
                            if (diff < 0 && notFound[0])
                            {
                                res += Dir::UP;
                                notFound[0] = false;
                            }
                            else if (diff > 0 && notFound[1])
                            {
                                res += Dir::DOWN;
                                notFound[1] = false;
                            }
                        }
                    }
                }
            }

            return res;
        }
};



class Point : public Figure
{
    private:
        bool iterEnd = false;


    public:
        char pointChar;
        Point(char pointChar, const Coord& vert = { 0, 0 }, const std::string& fgColor = "", const std::string& bgColor = "") : Figure(vert, fgColor, bgColor)
        {
            this->pointChar = pointChar;
            Draw();
        }


        void Clear(bool getCoord = true, bool getBg = false, bool getFg = false) override
        {
            bool dontOptimize = getCoord || getBg || getFg;

            if (dontOptimize)
                Scr::SaveState();

            Scr::SetColor("", Scr::SCREEN_BG);
            Scr::AtCoord(thisState.coord);
            outBuff << " ";

            if (dontOptimize)
                Scr::RetrieveState(getCoord, getBg, getFg);
        }


        void Draw(bool getCoord = true, bool getBg = false, bool getFg = false) override
        {
            bool dontOptimize = getCoord || getBg || getFg;
            
            if (dontOptimize)
                Scr::SaveState();
            
            Scr::UpdateState(thisState);
            outBuff << pointChar;

            if (dontOptimize)
                Scr::RetrieveState(getCoord, getBg, getFg);
        }


        void MoveTo(const Coord& where, bool getCoord = true, bool getBg = false, bool getFg = false) override
        {
            bool dontOptimize = getCoord || getBg || getFg;
            if (dontOptimize)
                Scr::SaveState();
            
            Clear(false);
            thisState.coord = where;
            Draw(false);
            
            if (dontOptimize)
                Scr::RetrieveState(getCoord, getBg, getFg);
        }


        void ChangeSeq(const std::string& seq) override
        {
            this->pointChar = seq[0];

            Scr::UpdateState(thisState);
            outBuff << pointChar;
        }


        Coord Next(bool reset = false) override
        {
            if (reset || iterEnd)
            {
                iterEnd = false;
                return Coord(-1, -1);
            }

            iterEnd = true;
            return thisState.coord;
        }
};



class HorzLine : public Figure
{
    public:
        std::string pattern;
        int length, iterIndex = 0;

        
        HorzLine(const std::string& pattern, int length = -1, Coord vertex = { 0, 0 }, const std::string& fgColor = "", const std::string& bgColor = "") : Figure(vertex, fgColor, bgColor)
        {
            this->pattern = pattern;
            this->length = (length == -1) ? pattern.length() : length;
            Draw();
        }


        static void MonoBgGradient(Coord where, int start, int end, char rgb, int step = 1, bool retreat = true, int maxCol = Scr::WIDTH)
        {
            Scr::AtCoord(where);
            int r, g, b;

            for (int i = start; i <= end && step > 0 || i >= end && step < 0; i += step)
            {
                r = (rgb == 'r' || rgb == 'w') ? i : 0;
                g = (rgb == 'g' || rgb == 'w') ? i : 0;
                b = (rgb == 'b' || rgb == 'w') ? i : 0;

                Scr::SetColor("", Fmt("%%d;%d;%d", r, g, b));
                outBuff << ' ';
            }

            if (retreat)
            {
               for (int i = end - 1; i >= start && step > 0 || i <= start && step < 0; i -= step)
                {
                    r = (rgb == 'r' || rgb == 'w') ? i : 0;
                    g = (rgb == 'g' || rgb == 'w') ? i : 0;
                    b = (rgb == 'b' || rgb == 'w') ? i : 0;

                    Scr::SetColor("", Fmt("%%d;%d;%d", r, g, b));
                    outBuff << ' ';

                    if (stateNow.coord.COL == maxCol) break;
                }
            }
        }

        
        static int MonoBgGradient(int start, int end, int step, bool retreat = true)
        {
            int count = 0;
            for (int i = start; i <= end && step > 0 || i >= end && step < 0; i += step)
                count++;
            if (retreat)
                for (int i = end - 1; i >= start && step > 0 || i <= start && step < 0; i -= step)
                    count++;

            return count;
        }


        void Clear(bool getCoord = true, bool getBg = false, bool getFg = false) override
        {
            bool dontOptimize = getCoord || getBg || getFg;

            if (dontOptimize)
                Scr::SaveState();

            Scr::SetColor("", Scr::SCREEN_BG);
            Scr::AtCoord(thisState.coord);
            outBuff << std::string(length, ' ');

            if (dontOptimize)
                Scr::RetrieveState(getCoord, getBg, getFg);
        }


        void Draw(bool getCoord = true, bool getBg = false, bool getFg = false) override
        {
            bool dontOptimize = getCoord || getBg || getFg;

            if (dontOptimize)
                Scr::SaveState();
            
            int pat_len = pattern.length();

            Scr::UpdateState(thisState);

            for (int i = 0; i < length; i++)
                outBuff << pattern[i % pat_len];

            if (dontOptimize)
                Scr::RetrieveState(getCoord, getBg, getFg);
        }


        void MoveTo(const Coord& where, bool getCoord = true, bool getBg = false, bool getFg = false) override
        {
            bool dontOptimize = getCoord || getBg || getFg;

            if (dontOptimize)
                Scr::SaveState();
            
            Clear(false);
            thisState.coord = where;
            Draw(false);

            if (dontOptimize)
                Scr::RetrieveState(getCoord, getBg, getFg);
        }


        void ChangeSeq(const std::string& pattern)  override
        {
            this->pattern = pattern;
            Draw(false);
        }


        Coord Next(bool reset = false) override
        {
            if (reset || iterIndex == length)
            {
                iterIndex = 0;
                return Coord(-1, -1);
            }

            return thisState.coord + Coord(0, iterIndex++);
        }


        ~HorzLine()
        {
            Clear();
        }
};



class VertLine : public Figure
{
    private:
        std::string pattern;
        int length, iterIndex = 0;

    public:
        VertLine(const std::string& pattern, int length = -1, Coord vertex = { 0, 0 }, const std::string& fgColor = "", const std::string& bgColor = "") : Figure(vertex, fgColor, bgColor)
        {
            this->pattern = pattern;
            this->length = (length == -1) ? pattern.length() : length;
            Draw();
        }


        void Clear(bool getCoord = true, bool getBg = false, bool getFg = false) override
        {
            bool dontOptimize = getCoord || getBg || getFg;

            if (dontOptimize)
                Scr::SaveState();
            
            Scr::SetColor("", Scr::SCREEN_BG);
            Scr::AtCoord(thisState.coord);

            for (int n = length; n--; )
            {
                outBuff << " ";
                Scr::MoveCursor(Dir::DOWN);
                Scr::MoveCursor(Dir::LEFT);
            }

            if (dontOptimize)
                Scr::RetrieveState(getCoord, getBg, getFg);
        }


        void Draw(bool getCoord = true, bool getBg = false, bool getFg = false) override
        {            
            int pat_len = pattern.length();

            bool dontOptimize = getCoord || getBg || getFg;

            if (dontOptimize)
                Scr::SaveState();
            
            
            Scr::UpdateState(thisState);
                
            for (int i = 0; i < length; i++)
            {
                outBuff << pattern[i % pat_len];
                Scr::MoveCursor(Dir::DOWN);
                Scr::MoveCursor(Dir::LEFT);
            }

            if (dontOptimize)
                Scr::RetrieveState(getCoord, getBg, getFg);
        }


        void MoveTo(const Coord& where, bool getCoord = true, bool getBg = false, bool getFg = false) override
        {
            bool dontOptimize = getCoord || getBg || getFg;

            if (dontOptimize)
                Scr::SaveState();

            Clear(false);
            thisState.coord = where;
            Draw(false);

            if (dontOptimize)
                Scr::RetrieveState(getCoord, getBg, getFg);
        }


        void ChangeSeq(const std::string& pattern) override
        {
            this->pattern = pattern;
            Draw(false);
        }


        Coord Next(bool reset = false) override
        {
            if (reset || iterIndex == length)
            {
                iterIndex = 0;
                return Coord(-1, -1);
            }

            return thisState.coord + Coord(iterIndex++, 0);
        }


        ~VertLine()
        {
            Clear();
        }
};



class Block : public Figure
{
    private:
        std::string pattern;
        int iterIndex = 0;

    public:
        int width, height;

        
        Block(const std::string& pattern, int width, int height = -1, Coord vertex = { 0, 0 }, const std::string& bgColor = "", const std::string& fgColor = "") : Figure(vertex, fgColor, bgColor)
        {
            this->pattern = pattern;
            this->width = width;
            this->height = (height == -1) ? width : height;
            Draw();
        }


        void Clear(bool getCoord = true, bool getBg = false, bool getFg = false) override
        {
            bool dontOptimize = getCoord || getBg || getFg;

            if (dontOptimize)
                Scr::SaveState();
            
            Scr::SetColor("", Scr::SCREEN_BG);
            Scr::AtCoord(thisState.coord);

            for (int n = height; n--; )
            {
                outBuff << std::string(width, ' ');
                Scr::MoveCursor(Dir::DOWN);
                Scr::MoveCursor(Dir::LEFT, width);
            }

            if (dontOptimize)
                Scr::RetrieveState(getCoord, getBg, getFg);
        }


        void Draw(bool getCoord = true, bool getBg = false, bool getFg = false) override
        {            
            int pat_len = pattern.length();

            bool dontOptimize = getCoord || getBg || getFg;

            if (dontOptimize)
                Scr::SaveState();
            

            Scr::UpdateState(thisState);

            for (int n = height; n--; )
            {
                if (pattern.length() == 1)
                {
                    for (int i = 0; i < width; i++)
                        outBuff << pattern[i % pat_len];
                }
                else
                {
                    outBuff << std::string(width, pattern[0]);
                }

                Scr::MoveCursor(Dir::DOWN);
                Scr::MoveCursor(Dir::LEFT, width);
            }

            if (dontOptimize)
                Scr::RetrieveState(getCoord, getBg, getFg);
        }


        void MoveTo(const Coord& where, bool getCoord = true, bool getBg = false, bool getFg = false) override
        {
            bool dontOptimize = getCoord || getBg || getFg;

            if (dontOptimize)
                Scr::SaveState();

            Clear(false);
            thisState.coord = where;
            Draw(false);

            if (dontOptimize)
                Scr::RetrieveState(getCoord, getBg, getFg);
        }


        // RETURNS FALSE IF CAN'T EXPAND/CONTRACT FURTHER
        bool Reframe(const Coord& delta, bool getCoord = true, bool getBg = false, bool getFg = false)
        {
            int oldWidth = width;
            height += delta.ROW;
            width  += delta.COL;

            if (height <= 0 || width <= 0 || height > Scr::HEIGHT || width > Scr::WIDTH)
                return false;

            bool notOptimize = !(getCoord || getBg || getFg);

            if (notOptimize)
                Scr::SaveState();
            
            int pat_len = pattern.length();
            int minWidth  = std::min(width,  width  - delta.COL);
            int minHeight = std::min(height, height - delta.ROW);


            if (delta.COL > 0)
            {
                Scr::SetColor(thisState.fgColor, thisState.bgColor);

                for (int n = 0; n < minHeight; n++ )
                {
                    Scr::AtCoord({ thisState.coord.ROW + n, thisState.coord.COL + minWidth });
                    for (int i = 0; i < delta.COL; i++)
                        outBuff << pattern[i % pat_len];
                }
            }
            else
            {
                Scr::SetColor("", Scr::SCREEN_BG);

                for (int n = 0; n < minHeight; n++ )
                {
                    Scr::AtCoord({ thisState.coord.ROW + n, thisState.coord.COL + minWidth });
                    outBuff << std::string(-delta.COL, ' ');
                }
            }
    
            if (delta.ROW > 0)
            {
                Scr::SetColor(thisState.fgColor, thisState.bgColor);

                for (int n = minHeight; n < height; n++)
                {
                    Scr::AtCoord({ thisState.coord.ROW + n, thisState.coord.COL });
                    for (int i = 0; i < width; i++)
                        outBuff << pattern[i % pat_len];
                }
            }
            else
            {
                Scr::SetColor("", Scr::SCREEN_BG);

                for (int n = 0; n < -delta.ROW; n++)
                {
                    Scr::AtCoord({ thisState.coord.ROW + minHeight + n, thisState.coord.COL });
                    outBuff << std::string(oldWidth, ' ');
                }
            }

            if (notOptimize)
                Scr::RetrieveState(getCoord, getBg, getFg);

            return true;
        }


        void ChangeSeq(const std::string& pattern) override
        {
            this->pattern = pattern;
            Draw(false);
        }


        Coord Next(bool reset = false) override
        {
            if (reset || iterIndex == 2 * (width + height) - 4)
            {
                iterIndex = 0;
                return Coord(-1, -1);
            }
            
            Coord offset;

            if (iterIndex < width)
                offset = { 0, iterIndex };

            else if (iterIndex < width + height - 1)
                offset = { iterIndex - width + 1, width - 1 };

            else if (iterIndex < 2 * (width - 1) + height)
                offset = { height - 1, 2 * width + height - iterIndex - 3 };

            else
                offset = { 2 * (width + height) - iterIndex - 4, 0 };


            iterIndex++;
            return Coord(thisState.coord + offset);
        }


        ~Block()
        {
            Clear();
        }
};



class Group : public Figure
{
    protected:
        std::vector<Figure*> elms;
        int anchor = -1, iterIndex = 0;
    
    
    public:
        Group(std::vector<Figure*> vec, int anchor = -1)
        {
            elms = vec;
            this->anchor = anchor;
        }


        Group(Figure *arr[], int length, int anchor = -1)
        {
            for (int i = 0; i < length; i++)
                elms.push_back(arr[i]);
            
            this->anchor = anchor;
        }


        std::vector<Figure*> GetElements()
        {
            return elms;
        }


        void Clear(bool getCoord = true, bool getBg = false, bool getFg = false) override
        {
            bool dontOptimize = getCoord || getBg || getFg;

            if (dontOptimize)
                Scr::SaveState();

            for (Figure* elm : elms)
                elm->Clear(false);

            if (dontOptimize)
                Scr::RetrieveState(getCoord, getBg, getFg);
        }


        void ReDraw(bool getCoord = true, bool getBg = false, bool getFg = false)
        {
            MoveBy({0, 0}, getCoord, getBg, getFg);
        }


        void MoveBy(const Coord& diff, bool getCoord = true, bool getBg = false, bool getFg = false) override
        {
            bool dontOptimize = getCoord || getBg || getFg;

            if (dontOptimize)
                Scr::SaveState();

            Clear(false);

            for (Figure* elm : elms)
            {
                elm->thisState.coord += diff;
                elm->Draw(false);
            }

            if (dontOptimize)
                Scr::RetrieveState(getCoord, getBg, getFg);
        }


        void MoveTo(const Coord& dest, bool getCoord = true, bool getBg = false, bool getFg = false) override
        {
            MoveBy(dest - elms[anchor]->thisState.coord, getCoord, getBg, getFg);
        }


        void ChangeColor(const std::string& fgColor = "", const std::string& bgColor = "", bool optimize = false) override
        {
            if (!optimize)
                Scr::SaveState();

            for (Figure* elm : elms)
                elm->ChangeColor(fgColor, bgColor, false);

            if (!optimize)
                Scr::RetrieveState();
        }


        Coord Next(bool reset = false) override
        {
            while (true)
            {
                if (reset || iterIndex == elms.size())
                {
                    if (iterIndex)
                    {
                        elms[reset ? iterIndex : iterIndex - 1]->Next(true);
                        iterIndex = 0;
                    }

                    return Coord(-1, -1);
                }

                Coord ptNow = elms[iterIndex]->Next();

                if (!ptNow.isValid())
                {
                    iterIndex++;
                    continue;
                }

                return ptNow;
            }
        }
};



class Grid
{
    private:
        char horzChar, vertChar, juncChar;
        std::string borderType = "both";
        

        void DrawHorz(char juncChar)
        {
            if (horzChar == juncChar)
            {
                outBuff << std::string((colspan + 1) * col + 1, horzChar);
                return;
            }
            outBuff << juncChar;

            for (int n = col; n--; ) {
                outBuff << std::string(colspan, horzChar) << juncChar;
            }
        }


        void DrawVert(char vertChar, bool skipEnds)
        {
            if (skipEnds)
            {
                Scr::SetColor("", data[0].style.bgColor);
                outBuff << ' ';
            }
            else {
                outBuff << vertChar;
            }

            for (int n = col; n--; ) {
                Scr::SetColor("", Scr::SCREEN_BG);
                outBuff << std::string(colspan, ' ');
                Scr::SetColor("", borderStyle.bgColor);
                
                if (skipEnds && n == 0)
                {
                    Scr::SetColor("", data.back().style.bgColor);
                    outBuff << ' ';
                }
                else
                    outBuff << vertChar;
            }
        }



    public:
        class StyledText
        {
            public:
                std::string text;
                State style;

                StyledText(std::string text, State style) : text(text), style(style) {  }
        };


        bool skipEnds = false;
        State borderStyle;
        std::vector<StyledText> data;
        int row, col, colspan, rowspan;


        Grid(const Coord& vertex = { 0, 0 }, std::string borderFg = "", std::string borderBg = "", char horzChar = '-', char vertChar = '|', char juncChar = '+') : horzChar(horzChar), vertChar(vertChar), juncChar(juncChar)
        {
            if (borderFg == "")
                borderFg = "WHITE";
            if (borderBg == "")
                borderBg = Scr::SCREEN_BG;

            borderStyle = { ParseCoord(vertex), borderFg, borderBg };
        }


        void Configure(int row, int col, int rowspan = 4, int colspan = 9, const std::string& borderType = "both", const std::string& textBg = Scr::SCREEN_BG, const std::string& textFg = "WHITE", const std::string& style = "NONE")
        {
            this->row = row;
            this->col = col;
            this->rowspan = rowspan;
            this->colspan = colspan;
            this->borderType = borderType;

            for (int i = 0; i < row * col; i++)
            {
                int r = (i / col) + 1, c = (i % col) + 1;

                Coord cell_st = borderStyle.coord + (borderType == "both" ? Coord(1, 1) : borderType == "horz" ? Coord(1, 0) : borderType == "vert" ? Coord(0, 1) : Coord(0, 0));
                cell_st.ROW += (borderType == "both" || borderType == "horz") ? (rowspan + 1) * (r - 1) : rowspan * (r - 1);
                cell_st.COL += (borderType == "both" || borderType == "vert") ? (colspan + 1) * (c - 1) : colspan * (c - 1);


                data.push_back( StyledText("", State(cell_st, textFg, textBg, style)) );
            }
        }


        void Draw()
        {
            Scr::SaveState();
            Scr::UpdateState(borderStyle);

            if (borderType == "both") {
                DrawHorz(juncChar);
            }
            else if (borderType == "horz") {
                DrawHorz('\0');
            }

            int n_left = (borderType == "both" || borderType == "vert") ? (colspan + 1) * col + 1 : colspan * col;

            for (int n = row; n--; )
            {
                if (borderType == "both" || borderType == "horz")
                {
                    Scr::MoveCursor(Dir::DOWN);
                    Scr::MoveCursor(Dir::LEFT, n_left);
                }
                    
                for (int k = rowspan; k--; )
                {
                    DrawVert((borderType == "both" || borderType == "vert") ? vertChar : '\0', skipEnds);

                    Scr::MoveCursor(Dir::DOWN);
                    Scr::MoveCursor(Dir::LEFT, n_left);
                }

                if (borderType == "both") {
                    DrawHorz(juncChar);
                }
                else if (borderType == "horz") {
                    DrawHorz('\0');
                }
            }
            
            Render();
            Scr::RetrieveState(true, true, true, true);
            outBuff.flush();
        }


        void Render()
        {
            for (int r = 1; r <= row; r++)
            {
                for (int c = 1; c <= col; c++)
                {
                    StyledText& textObject = data[col * (r - 1) + c - 1];
                    std::string& text = textObject.text;
                    
                    Scr::UpdateState(textObject.style);

                    std::vector<std::string> lines;
                    std::stringstream ss(text);
                    std::string token;

                    while (std::getline(ss, token, '\n')) {
                        lines.push_back(token);
                    }

                    int rowOffset = (rowspan - lines.size()) / 2;
                    
                    for (int i = 0; i < rowOffset; i++)
                    {
                        outBuff << std::string(colspan, ' ');
                        Scr::MoveCursor(Dir::DOWN);
                        Scr::MoveCursor(Dir::LEFT, colspan);
                    }

                    for (std::string line : lines)
                    {
                        outBuff << Center(line, colspan);
                        Scr::MoveCursor(Dir::DOWN);
                        Scr::MoveCursor(Dir::LEFT, colspan);
                    }

                    for (int i = rowspan - rowOffset - lines.size(); i--; )
                    {
                        outBuff << std::string(colspan, ' ');
                        Scr::MoveCursor(Dir::DOWN);
                        Scr::MoveCursor(Dir::LEFT, colspan);
                    }
                }
            }

            outBuff.flush();
        }


        void Clear()
        {
            Scr::SaveState();
            Scr::SetColor("", Scr::SCREEN_BG);

            int n_left = (borderType == "both" || borderType == "vert") ? (colspan + 1) * col + 1 : colspan * col;
            int n = (borderType == "both" || borderType == "horz") ? (rowspan + 1) * row : rowspan * row;

            while (n--)
            {
                outBuff << std::string(n_left, ' ');

                Scr::MoveCursor(Dir::DOWN);
                Scr::MoveCursor(Dir::LEFT, n_left);
            }

            Scr::RetrieveState(true, true, true, true);
        }


        StyledText* GetAccess(Coord coord)
        {
            return &data[col * (coord.ROW - 1) + coord.COL - 1];
        }


        void Update(Coord coord, std::string text, std::string fgColor = "", std::string bgColor = "", std::string style = "NONE")
        {
            if (text.length() > colspan) return;

            StyledText& textObject = data[col * (coord.ROW - 1) + coord.COL - 1];

            if (text != "") {
                textObject.text = text;
            }
            if (fgColor != "") {
                textObject.style.fgColor = fgColor;
            }
            if (bgColor != "") {
                textObject.style.bgColor = bgColor;
            }

            textObject.style.style = style;
        }


        // frameStyle.bgColor = ""  =>  hollow frame
        static Grid* MakeLabel(std::string text, State textStyle, Coord topLeft = {-1, -1}, int colPad = 3, int rowPad = 1, State frameStyle = { })
        {
            if (topLeft == Coord(-1, -1)) topLeft = stateNow.coord;

            Grid* label;

            if (frameStyle.fgColor == "")
                label = new Grid(topLeft, "", frameStyle.bgColor, ' ', ' ', ' ');
            else
                label = new Grid(topLeft, frameStyle.fgColor, frameStyle.bgColor);

            label->Configure(1, 1, rowPad * 2 + 1, text.length() + colPad * 2, (frameStyle.fgColor == "" && frameStyle.bgColor == "") ? "none" : "both");
            label->Update({1, 1}, text, textStyle.fgColor, textStyle.bgColor);
            label->Draw();
            return label;
        }
};



class Button
{
    private:
        int refAfter, moveAfter, refCount = 0, moveCounter = 0;
        Grid* grid;
        bool noHighlight;


    public:
        static const Coord BACKSPACE;
        static const Coord RIGHT_CLICK;
        static const Coord ESCAPE;
        State selStyle, prevStyle;
        Coord selCoord = { 1, 1 };


        Button(Grid* grid, State selStyle = { }, int refAfter = 5, int moveAfter = 30, bool noHighlight = false) : refAfter(refAfter), moveAfter(moveAfter), grid(grid), selStyle(selStyle), noHighlight(noHighlight)
        {
            auto cell = grid->GetAccess(selCoord);
            prevStyle = cell->style;

            if (!noHighlight)
            {
                cell->style.fgColor = selStyle.fgColor;
                cell->style.bgColor = selStyle.bgColor;
                cell->style.style = selStyle.style;
            }

            grid->Draw();
        }


        Coord Check()
        {
            if (moveCounter)  moveCounter--;
            else if (refCount++ == refAfter)
            {
                refCount = 0;
                int key;

                if (key = GetKey())
                {
                    if (!noHighlight)
                    {
                        auto cell = grid->GetAccess(selCoord);

                        cell->style.fgColor = prevStyle.fgColor;
                        cell->style.bgColor = prevStyle.bgColor;
                        cell->style.style = prevStyle.style;
                    }


                    switch (key)
                    {
                        case VK_LEFT:
                            if (selCoord.COL > 1) selCoord.COL--;
                            else selCoord.COL = grid->col;
                            break;

                        case VK_RIGHT:
                            if (selCoord.COL < grid->col) selCoord.COL++;
                            else selCoord.COL = 1;
                            break;
                            
                        case VK_DOWN:
                            if (selCoord.ROW < grid->row) selCoord.ROW++;
                            else selCoord.ROW = 1;
                            break;

                        case VK_UP:
                            if (selCoord.ROW > 1) selCoord.ROW--;
                            else selCoord.ROW = grid->row;
                            break;
                    }


                    if (VK_LEFT <= key && key <= VK_DOWN)
                        moveCounter = moveAfter;
                    

                    if (!noHighlight)
                    {
                        auto cell = grid->GetAccess(selCoord);

                        prevStyle = cell->style;
                        cell->style.fgColor = selStyle.fgColor;
                        cell->style.bgColor = selStyle.bgColor;
                        cell->style.style = selStyle.style;
                    }

                    grid->Render();

                    if (key == VK_RETURN)  return selCoord;
                    else if (key == VK_BACK)  return BACKSPACE;
                    else if (key == VK_ESCAPE)  return ESCAPE;
                    else if (key == VK_RBUTTON)  return RIGHT_CLICK;
                }
            }

            return Coord();
        }


        void NoHighlight(bool ans)
        {
            noHighlight = ans;

            auto cell = grid->GetAccess(selCoord);

            if (!ans)
            {
                prevStyle = cell->style;
                
                cell->style.fgColor = selStyle.fgColor;
                cell->style.bgColor = selStyle.bgColor;
                cell->style.style = selStyle.style;
            }
            else
            {
                cell->style.fgColor = prevStyle.fgColor;
                cell->style.bgColor = prevStyle.bgColor;
                cell->style.style = prevStyle.style;
            }
        }
};



class Entry
{
    public:
        static std::string TakeInput(Grid& textArea, Coord where = {1, 1}, const std::string& prompt = " ", const std::string& fgColor = "", const std::string& bgColor = "", const std::string& type = "all")
        {
            int key;
            std::string buff;
            bool shiftOn = false;
            bool capsOn = false;

            textArea.Update(where, prompt, fgColor, bgColor);
            textArea.Render();

            int penalty = 0;
            int blinkCounter = 0;
            bool toggle = false;

            while ((key = GetKey()) != VK_RETURN || buff == "")
            {
                if (!blinkCounter)
                {
                    blinkCounter = 20;
                    if (buff != "")
                    {
                        textArea.Update(where, (toggle) ? buff : buff.substr(0, buff.length() - 1));
                        textArea.Render();
                    }
                    toggle = !toggle;
                }
                else blinkCounter--;

                if (penalty)
                {
                    penalty--;
                    MicroSleep(10 MS);
                    continue;
                }

                if (!key)
                {
                    MicroSleep(30 MS);
                    continue;
                }
                if (key == VK_SHIFT)
                {
                    shiftOn = true;
                    MicroSleep(60 MS);
                    continue;
                }
                
                key = MapVirtualKey(key, MAPVK_VK_TO_CHAR);

                if (shiftOn)
                {
                    capsOn = !capsOn;
                    shiftOn = false;
                    if (shifted.count(key))
                        key = shifted[key];
                }

                if (type == "char" && 32 <= key && key <= 126)
                    return std::string(1, key);
                    
                if (type == "all" && 32 <= key && key <= 126 || type == "alpha" && isalpha(key) || type == "digit" && isdigit(key))
                {
                    if (isalpha(key) && !capsOn)
                        key += 32;
                    buff += key;
                    textArea.Update(where, buff);
                    textArea.Render();
                    penalty = 12;
                }
                else if (key == VK_BACK && !buff.empty())
                {
                    buff.pop_back();
                    textArea.Update(where, (buff == "" ? " " : buff));
                    textArea.Render();
                    penalty = 8;
                }

                MicroSleep(10 MS);
            }

            textArea.Update(where, buff);
            textArea.Render();
            return buff;
        }
};



class Pixel
{
    public:
        char c, fg[10], bg[10];
        byte id = 0;


        Pixel(char c = ' ', std::string fgColor = "WHITE", std::string bgColor = "BLACK") : c(c)
        {
            strcpy(fg, fgColor.c_str());
            strcpy(bg, bgColor.c_str());
        }


        void Print()
        {
            Scr::SetColor(fg, bg);
            outBuff << c;
        }


        bool operator==(Pixel another)
        {
            return c == another.c && !strcmp(fg, another.fg) && !strcmp(bg, another.bg) && id == another.id;
        }
};



class PixelGrid
{
    private:
        int row, col;
        Coord vertex;
        std::vector<Pixel> panel;


    public:
        PixelGrid(int rows, int cols, Coord vertex = { 0, 0 }) : row(rows), col(cols) {
            this->vertex = ParseCoord(vertex);
            panel.resize(rows * cols);
        }


        // only changes array data
        void Add(Coord where, Pixel pixel)
        {
            int row = vertex.ROW + where.ROW - 1;
            int col = vertex.COL + where.COL - 1;

            GetPixel({row, col}) = pixel;
        }


        // only changes display
        void Clear(byte id)
        {
            Scr::SetColor("", Scr::SCREEN_BG);

            for (int r = 1; r <= row; r++)
            {
                for (int c = 1; c <= col; c++)
                {
                    Pixel pixel = GetPixel({r, c});

                    if (pixel.id == id)
                    {
                        Scr::AtCoord({vertex.ROW + r - 1, vertex.COL + c - 1});
                        outBuff << ' ';
                    }
                }
            }
        }


        void Draw(byte id)
        {
            Scr::SetColor("", Scr::SCREEN_BG);

            for (int r = 1; r <= row; r++)
            {
                for (int c = 1; c <= col; c++)
                {
                    Pixel& pixel = GetPixel({r, c});

                    if (pixel.id == id)
                    {
                        Scr::AtCoord({vertex.ROW + r - 1, vertex.COL + c - 1});
                        pixel.Print();
                    }
                }
            }
        }


        // does both
        void Move(byte id, Dir::Direction where, int n = 1)
        {
            Clear(id);
            std::vector<Coord> moved;

            for (int r = 1; r <= row; r++)
            {
                for (int c = 1; c <= col; c++)
                {
                    Pixel& pixel = GetPixel({r, c});

                    if (pixel.id == id && std::find(moved.begin(), moved.end(), Coord(r, c)) == moved.end())
                    {
                        int rowDest = vertex.ROW + r - 1;
                        int colDest = vertex.COL + c - 1;
                        
                        switch (where)
                        {
                            case Dir::RIGHT:
                                colDest += n;
                                break;
                            
                            case Dir::LEFT:
                                colDest -= n;
                                break;
                            
                            case Dir::UP:
                                rowDest -= n;
                                break;
                            
                            case Dir::DOWN:
                                rowDest += n;
                        }

                        if (rowDest > row || colDest > col)
                            continue;

                        panel[col * (rowDest - 1) + colDest - 1] = pixel;

                        Scr::AtCoord({rowDest, colDest});
                        pixel.Print();

                        pixel.c = ' ';
                        pixel.fg[0] = '\0';
                        pixel.bg[0] = '\0';
                        pixel.id = 0;

                        moved.push_back(Coord(r, c));
                    }
                }
            }
        }


        void Render()
        {
            for (int r = 1; r <= row; r++)
            {
                for (int c = 1; c <= col; c++)
                {
                    Pixel& pixel = GetPixel({r, c});

                    Scr::AtCoord({ vertex.ROW + r - 1, vertex.COL + c - 1 });
                    pixel.Print();
                }
            }
        }


        Pixel& GetPixel(Coord coord)
        {
            return panel[col * (coord.ROW - 1) + coord.COL - 1];
        }
};



class PixelMap
{
    private:
        std::vector<std::pair<Coord, Pixel>> assoc;

    
    public:
        PixelMap(std::vector<typename std::pair<Coord, Pixel>> map) : assoc(map) {  }


        void Display()
        {
            for (auto pair : assoc)
            {
                Scr::AtCoord(pair.first);
                pair.second.Print();
            }
        }


        Group* GetGroup()
        {
            std::vector<Figure*> elms;
            
            for (auto elm : assoc)
                elms.push_back(new Point(elm.second.c, elm.first, elm.second.fg, elm.second.bg));

            return new Group(elms);
        }


        void MoveBy(const Coord& by)
        {
            Scr::SetColor("", Scr::SCREEN_BG);

            for (auto& elm : assoc)
            {
                Scr::AtCoord(elm.first);
                elm.first += by;
                outBuff << ' ';
            }

            for (auto elm : assoc)
            {
                Scr::AtCoord(elm.first);
                elm.second.Print();
            }
        }
};