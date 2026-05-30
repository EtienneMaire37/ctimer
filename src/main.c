#include <ncurses.h>
#include <panel.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>

int main()
{
    atexit((void*)endwin);
    for (int i = 0; i < SIGRTMIN; i++)
        signal(i, exit);

    initscr();

    while (true);

    endwin();
}
