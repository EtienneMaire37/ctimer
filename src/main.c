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

bool setup_rendering_and_xml_data = false;
xml_tag_t* first_tag;
xml_tag_t *GameName, *CategoryName, *FirstSegment;

void print_at(int x, int y, char (*filter)(int, int, char), const char* fmt, ...)
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
}

char simple_filter(int x, int y, char ch)
{
    if (x < 1 || y < 1 || x > COLS - 2 || y > LINES - 2) return 0;
    if (x == COLS - 2) return '-';
    return ch;
}

void sigwinch()
{
    if (!setup_rendering_and_xml_data) return;

    endwin();
    initscr();

    clear();

    border(0, 0, 0, 0, 0, 0, 0, 0);
    print_at(2, 1, simple_filter, "%s - %s", GameName->data, CategoryName->data);
    int seg = 0;
    xml_tag_t* Segment = FirstSegment;
    while (Segment && seg < LINES - 7)
    {
        if (Segment->in)
            print_at(3, 3 + seg, simple_filter, "* %s", Segment->in->data);
        Segment = Segment->next;
        seg++;
    }

    if (Segment && LINES > 7)
    {
        while (Segment->next)
            Segment = Segment->next;
        if (Segment->in)
            print_at(3, LINES - 3, simple_filter, "* %s", Segment->in->data);
    }

    refresh();
}

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

    first_tag = xml_load_from_text(xml_data);

    GameName = xml_get_tag(first_tag, "Run/GameName");
    if (!GameName)
    {
        fprintf(stderr, "Invalid .lss file!");
        return 3;
    }
    CategoryName = xml_get_tag(first_tag, "Run/CategoryName");
    if (!CategoryName)
    {
        fprintf(stderr, "Invalid .lss file!");
        return 3;
    }
    FirstSegment = xml_get_tag(first_tag, "Run/Segments/Segment");
    if (!FirstSegment)
    {
        fprintf(stderr, "No segments!");
        return 3;
    }

    atexit((void*)endwin);
    setup_signals();
    signal(SIGWINCH, sigwinch);

    initscr();
    noecho();
    cbreak();
    curs_set(0);

    setup_rendering_and_xml_data = true;

    sigwinch();

    while (true)
        pause();

    endwin();
}
