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
    unsigned int minutes = (unsigned int)(time / 60) % 60, seconds = (unsigned int)time % 60, milliseconds = (unsigned int)(time * 1000) % 1000;
    char buffer[64] = {0};
    int size = snprintf(buffer, sizeof(buffer) - 1, " %2u:%02u.%02u", minutes, seconds, milliseconds / 10);
    print_at(COLS - size - 2, y, simple_filter, "%s", buffer);
}

char simple_filter(int x, int y, char ch)
{
    if (x < 1 || y < 1 || x > COLS - 2 || y > LINES - 2) return 0;
    if (x == COLS - 2) return '-';
    return ch;
}
