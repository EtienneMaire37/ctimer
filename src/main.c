#include <ncurses.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#include "usage.h"
#include "signals.h"

int main(int argc, char** argv)
{
    FILE* input_file = NULL;
    if (argc != 2)
    {
        if (argc != 0 || isatty(STDIN_FILENO))
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

    struct stat st;
    if (fstat(fileno(input_file), &st) != 0)
    {
        perror("ctimer");
        return 2;
    }

    char* xml_data = malloc(st.st_size);
    if (!xml_data)
    {
        perror("ctimer");
        return 2;
    }
    fread(xml_data, st.st_size, 1, input_file);
    // fwrite(xml_data, st.st_size, 1, stdout);

    // return 0;

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
