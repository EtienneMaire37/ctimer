#include <ncurses.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>

#include "usage.h"
#include "signals.h"
#include "parser.h"

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

    xml_tag_t* first_tag = xml_load_from_text(xml_data);

    xml_tag_t* GameName = xml_get_tag(first_tag, "Run/GameName");
    if (!GameName)
    {
        fprintf(stderr, "Invalid .lss file!");
        return 3;
    }
    xml_tag_t* CategoryName = xml_get_tag(first_tag, "Run/CategoryName");
    if (!CategoryName)
    {
        fprintf(stderr, "Invalid .lss file!");
        return 3;
    }
    xml_tag_t* Segment = xml_get_tag(first_tag, "Run/Segments/Segment");
    if (!Segment)
    {
        fprintf(stderr, "No segments!");
        return 3;
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
    mvaddstr(1, 2, GameName->data);
    mvaddstr(1, 2 + strlen(GameName->data), " - ");
    mvaddstr(1, 2 + strlen(GameName->data) + 3, CategoryName->data);
    int seg = 0;
    while (Segment && seg < 8)
    {
        mvaddstr(3 + seg, 3, "* ");
        if (strlen(Segment->in->data) >= 2 * 12 - 5 - 3 + 1)
        {
            Segment->in->data[2 * 12 - 5 - 4] = '-';
            Segment->in->data[2 * 12 - 5 - 3] = 0;
        }
        if (Segment->in) mvaddstr(3 + seg, 3 + 2, Segment->in->data);
        Segment = Segment->next;
        seg++;
    }

    if (Segment && Segment->next)
    {
        while (Segment->next)
            Segment = Segment->next;
        mvaddstr(3 + 9, 3, "* ");
        if (strlen(Segment->in->data) >= 2 * 12 - 5 - 3 + 1)
        {
            Segment->in->data[2 * 12 - 5 - 4] = '-';
            Segment->in->data[2 * 12 - 5 - 3] = 0;
        }
        if (Segment->in) mvaddstr(3 + 9, 3 + 2, Segment->in->data);
    }

    refresh();
    while (true)
        ;

    endwin();
}
