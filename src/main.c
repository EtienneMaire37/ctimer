#include <ncurses.h>
#include <panel.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>

void sigwinch()
{
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
    mvaddstr(1, 2, "TITLE");
    mvaddstr(3, 3, "- Split 1");
    mvaddstr(4, 3, "- Split 2");
    mvaddstr(5, 3, "- Split 3");
    mvaddstr(6, 3, "- Split 4");
    mvaddstr(7, 3, "- Split 5");

    refresh();
    while (true)
        ;

    endwin();
}
