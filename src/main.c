#include <ncurses.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "usage.h"
#include "signals.h"

int main(int argc, char** argv)
{
    FILE* input_file = NULL;
    if (argc != 2)
    {
        if (isatty(STDIN_FILENO))
        {
            print_usage();
            return 1;
        }
        input_file = stdin;
    }
    if (!input_file)
    {
        input_file = fopen(argv[1], "rb");
        if (!input_file)
        {
            char err_str[33] = {0};
            snprintf(err_str, sizeof(err_str) - 1, "ctimer: `%s`", argv[1]);
            perror(err_str);
            return 1;
        }
    }

    atexit((void*)endwin);
    setup_signals();

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
