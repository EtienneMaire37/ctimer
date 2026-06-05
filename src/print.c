#include "print.h"
#include <ncurses.h>

int print_at(int x, int y, char (*filter)(int, int, char), const char* fmt, ...)
{
    char buffer[BUFSIZ];

    va_list list;
    va_start(list, fmt);

    int written = vsnprintf(buffer, sizeof(buffer) - 1, fmt, list);

    va_end(list);

    for (int i = 0; i < written; x++, i++)
    {
        char flt = filter(x, y, buffer[i]);
        if (!flt)
            continue;

        mvaddch(y, x, flt);
    }

    return written;
}

void print_time_at(int y, float time)
{
    unsigned int hours = (unsigned int)(time / 3600), minutes = (unsigned int)(time / 60) % 60, seconds = (unsigned int)time % 60, milliseconds = (unsigned int)(time * 1000) % 1000;
    char buffer[64] = {0};
    int size = 0;
    bool print_all = false;
    if (hours)
    {
        size += snprintf(&buffer[size], sizeof(buffer) - 1, "%u:", hours);
        print_all = true;
    }
    if (minutes || print_all)
    {
        size += snprintf(&buffer[size], sizeof(buffer) - 1, "%0*u:", print_all ? 2 : 1, minutes);
        print_all = true;
    }
    size += snprintf(&buffer[size], sizeof(buffer) - 1, "%0*u.%02u", print_all ? 2 : 1, seconds, milliseconds / 10);
    print_at(COLS - size - 3, y, simple_filter, " ");
    print_at(COLS - size - 2, y, simple_filter, "%s", buffer);
}

char simple_filter(int x, int y, char ch)
{
    if (x < 1 || y < 1 || x > COLS - 2 || y > LINES - 2) return 0;
    if (x == COLS - 2) return '-';
    return ch;
}
char basic_filter(int x, int y, char ch)
{
    if (x < 2 || y < 2 || x > COLS - 3 || y > LINES - 3) return 0;
    return ch;
}
