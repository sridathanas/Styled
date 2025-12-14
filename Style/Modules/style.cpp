#include <stdarg.h>
#include <windows.h>
#include "C:\Users\User\Dropbox\VSCode\Cpp\Modules\style.h"


std::map<std::string, char> cursorCtrls = {
    { "UP", 'A' },
    { "DOWN", 'B' },
    { "RIGHT", 'C' },
    { "LEFT", 'D' }
};


std::map<std::string, std::string> colors = {
    { "RED", "255;0;0" },
    { "PARROT", "0;255;0" },
    { "BLUE", "0;0;255" },
    { "BLACK", "0;0;0" },
    { "WHITE", "255;255;255" },
    { "YELLOW", "255;255;00" },
    { "LIGHTYEL", "255;255;40" },
    { "FLESH", "255;87;51" },
    { "PURPLE", "205;65;225" },
    { "VIOLET", "43;0;87" },
    { "PINK", "255;0;125" },
    { "DARKCYAN", "0;150;150" },
    { "CYAN", "0;255;255" },
    { "ROSE", "150;0;75" },
    { "EMERALD", "0;200;105" },
    { "OCHRE", "204;119;34" },
    { "POOP", "101;67;33" },
    { "SKY", "180;140;255" },
    { "LAVENDER", "180;100;255" },
    { "BATHROOM", "100;100;255" },
    { "GRAY", "32;32;32" },
    { "CONSOLE", "12;12;12" },
    { "BROWN", "110;40;15" },
    { "BURNTORNG", "255;40;0" },
    { "PASSION", "255;120;0" },
    { "DARKGREEN", "0;100;0" },
    { "PRUSSIAN", "0;0;160" },
    { "LOVE", "255;0;55" },
    { "COTCANDY", "210;155;210" }

};


std::map<std::string, std::string> styles = {
    { "H", "?25l" },
    { "UH", "?25h" },
    { "B", "1m" },
    { "UB", "22m" },
    { "I", "3m" },
    { "UI", "23m" },
    { "U", "4m" },
    { "UU", "24m" },
    { "S", "9m" },
    { "US", "29m" }
};


std::map<char, char> shifted = {
    { '`', '~' },
    { '1', '!' },
    { '2', '@' },
    { '3', '#' },
    { '4', '$' },
    { '5', '%' },
    { '6', '^' },
    { '7', '&' },
    { '8', '*' },
    { '9', '(' },
    { '0', ')' },
    { '-', '_' },
    { '=', '+' },
    { '[', '{' },
    { ']', '}' },
    { '\\', '|' },
    { ';', ':' },
    { '\'', '"' },
    { ',', '<' },
    { '.', '>' },
    { '/', '?' }
};



std::vector<State> Scr::LIFOSaves;
std::map<std::string, State> Scr::mapSaves;
std::string Scr::SCREEN_BG = "CONSOLE";

const Coord Button::BACKSPACE(0, 0);
const Coord Button::RIGHT_CLICK(-2, -2);
const Coord Button::ESCAPE(-3, -3);


int wait = 0;
OutBuffer outBuff(120);
State stateNow(Coord(1, 1), "WHITE", Scr::SCREEN_BG);


void Pause(int x)
{
    outBuff.flush();
    MicroSleep(x);
}


bool KeyDown(int key)
{
    return GetKeyState(key) & 0x8000;
}


int GetKey()
{
    for (int key = 1; key < 255; key++)
    {
        if (KeyDown(key))
            return key;
    }

    return 0;
}


void MicroSleep(long long microseconds)
{
    LARGE_INTEGER frequency;
    LARGE_INTEGER start, end;
    QueryPerformanceFrequency(&frequency);
    double ticksPerMicrosecond = static_cast<double>(frequency.QuadPart) / 1000000.0;
    QueryPerformanceCounter(&start);

    while (true) {
        QueryPerformanceCounter(&end);
        double elapsedMicroseconds = static_cast<double>(end.QuadPart - start.QuadPart) / ticksPerMicrosecond;

        if (elapsedMicroseconds >= microseconds) {
            break;
        }
    }
}


int CanvasDraw(const std::string& cmd, bool flush, bool is_line_start)
{
    int len = cmd.length();

    outBuff.IsLineStart(is_line_start);

    std::map<int, std::string> key_bindings;
    std::map<std::string, std::string> async_process;

    int i = 0;
    while (i < len)
    {
        int n = 0;
        char c = cmd[i];
        
        if (isspace(c))
        {
            i++;
            continue;
        }
        if (c == '\'' || c == '"')
        {
            std::string buff = "";

            while (cmd[++i] != '\'' && cmd[i] != '"')
                buff += cmd[i];

            if (i < len && cmd[++i] == '_')
                while (std::isdigit(cmd[++i]))
                    n = n * 10 + cmd[i] - 48;
            else
                n = 1;
            
            while (n--)
                outBuff << buff;
        }
        else if (c == 't')
        {
            while (++i < len && std::isdigit(cmd[i]))
                n = n * 10 + cmd[i] - 48;

            if (false)
                outBuff.flush();
            
            MicroSleep(n * 1000);
        }
        else if (isupper(c))
        {
            // key-bindings
            if (cmd[i+1] == ' ')
            {
                // K => [...]
                char key = cmd[i];

                int v_key = (
                    key == 'U' ? VK_UP :
                    key == 'D' ? VK_DOWN :
                    key == 'L' ? VK_LEFT :
                    key == 'R' ? VK_RIGHT :
                    key == 'S' ? VK_SPACE :
                    (int)key
                );

                i += 6;
                int depth = 1;
                std::string binding = "";

                while (depth > 1 || cmd[i] != ']')
                {
                    char ch = cmd[i];
                    depth += (ch == '[') ? 1 : (ch == ']') ? -1 : 0;
                    binding += ch;
                }

                key_bindings[v_key] = binding;
            }
            else
            {
                // color settings
                std::string data = std::string(1, c);

                while (++i < len && isalpha(cmd[i]))
                    data += cmd[i];


                if (colors.count(data) > 0)
                {
                    if (i < len && cmd[i] == '*')
                    {
                        Scr::SetColor("", data);
                        i++;
                    }
                    else {
                        Scr::SetColor(data);
                    }
                }
                else {
                    Scr::SetStyle(data);
                }
            }

        }
        else if (c == '-')   //reset styles
        {
            outBuff << "\033[0m";
            i++;
        }
        else if (c == 'h')
        {
            Scr::AtCoord();
            i++;
        }
        else if (c == 'b')
        {
            while (++i < len && std::isdigit(cmd[i]))
                n = n * 10 + cmd[i] - 48;

            outBuff << "\033[" << n << "D" << std::string(n, ' ');
        }
        else if (c == '(')
        {
            int coord[] = { 0, 0 };
            bool isFrac[] = { false, false };
            
            for (int j = 0; j < 2; j++)
            {
                if (cmd[i+1] == '.')
                {
                    isFrac[j] = true;
                    i++;
                }

                while (std::isdigit(cmd[++i]))
                    coord[j] = coord[j] * 10 + cmd[i] - 48;

                if (coord[j] == 0)
                    coord[j] = (j == 0) ? stateNow.coord.ROW : stateNow.coord.COL;
            }

            if (isFrac[0])
                coord[0] = (coord[0] * Scr::HEIGHT) / 100;

            if (isFrac[1])
                coord[1] = (coord[1] * Scr::WIDTH) / 100;

            Scr::AtCoord({ coord[0], coord[1] });
            i++;
        }
        else if (c == '{')
        {
            int depth = 1;
            std::string buff;

            while (true)
            {
                if (cmd[++i] == '\'' || cmd[i] == '"') {
                    buff += cmd[i];

                    while (cmd[++i] != '\'' && cmd[i] != '"')
                        buff += cmd[i];

                    buff += cmd[i++];
                }

                if (cmd[i] == '{')
                    depth++;
                else if (cmd[i] == '}' && --depth == 0)
                    break;
                
                buff += cmd[i];
            }

            if (++i < len && cmd[i] == '_')
            {
                while (++i < len && isdigit(cmd[i]))
                    n = n * 10 + cmd[i] - 48;
            }
            else {
                n = INT32_MAX;
            }

            while (n--)
            {
                if (CanvasDraw(buff, flush) == -1)
                    return -1;
            }
        }
        else
        {
            if (cmd[i+1] == '*')
            {
                n = 300;
                i++;
            }
            else
            {
                while (++i < len && std::isdigit(cmd[i]))
                    n = n * 10 + cmd[i] - 48;
            }
            
            if (!n) n = 1;

            int direction = c == 'u' ? Dir::UP :
                            c == 'd' ? Dir::DOWN :
                            c == 'r' ? Dir::RIGHT :
                            c == 'l' ? Dir::LEFT :
                            Dir::NONE;

            Scr::MoveCursor(direction, n);
        }


        if (KeyDown(VK_TAB)) {
            return -1;
        }


        if (flush)
            outBuff.flush();
        if (wait) MicroSleep(wait * 1000);
    }

    return 0;
}


std::string Fmt(const char *cmd, ...)
{
    std::string str = "";

    va_list args;
    va_start(args, cmd);

    for (int i = 0; cmd[i]; i++)
    {
        char c = cmd[i];

        if (c == '%')
        {
            c = cmd[++i];

            switch (c)
            {
                case 'd': {
                    str += std::to_string(va_arg(args, int));
                    break;
                }

                case 'c': {
                    str += va_arg(args, int);
                    break;
                }

                case 's': {
                    str += va_arg(args, char *);
                }

                default: {
                    str += cmd[--i];
                }
            }
        }
        else
            str += c;
    }

    va_end(args);
    return str;
}


std::string Center(std::string string, int width, char padChar)
{
    int len = string.length();
    int lpad = (width - len) / 2;
    int rpad = width - lpad - len;

    return std::string(lpad, padChar) + string + std::string(rpad, padChar);
}


Coord ParseCoord(const Coord& coord)
{
    int row = coord.ROW, col = coord.COL;
    
    return Coord( (row == 0) ? stateNow.coord.ROW : (row < 0) ? Scr::HEIGHT + row + 1 : row,
                (col == 0) ? stateNow.coord.COL : (col < 0) ? Scr::WIDTH + col + 1 : col );
}


std::string ColorSweep(int delayMs)
{
    static auto it = colors.begin();
    
    if (it == colors.end())
    {
        it = colors.begin();
        return "";
    }

    if (delayMs != -1) MicroSleep(delayMs MS);
    
    return (it++)->first;
}


int Ranint(int st, int end)
{
    return st + rand() % (end - st + 1);
}
