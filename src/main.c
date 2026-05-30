#include <ncurses.h>
#include <panel.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>

void sigwinch()
{
    border(0, 0, 0, 0, 0, 0, 0, 0);
    refresh();
}

int main()
{
    atexit((void*)endwin);
    for (int i = 0; i < SIGRTMIN; i++)
        signal(i, exit);
    signal(SIGWINCH, sigwinch);

    WINDOW* scr = initscr();
    noecho();
    cbreak();
    curs_set(0);
    // * do NOT use resizeterm as it modifies the SIGWINCH handler
    resize_term(15, 2 * 12);

    border(0, 0, 0, 0, 0, 0, 0, 0);
    refresh();
    while (true)
        ;

    endwin();
}
