#include <linux/input-event-codes.h>
#include <signal.h>
#include <sys/select.h>
#include <time.h>
#include <ncurses.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <stdatomic.h>

#include "usage.h"
#include "signals.h"
#include "parser.h"
#include "input.h"
#include "timer.h"
#include "print.h"

bool setup_rendering_and_xml_data = false;
xml_tag_t* first_tag;
xml_tag_t *GameName, *CategoryName, *FirstSegment;

struct timespec last_time = {0};

atomic_flag sigwinch_test = ATOMIC_FLAG_INIT;

void sigwinch(int sig)
{
    if (!setup_rendering_and_xml_data) return;

    if (atomic_flag_test_and_set(&sigwinch_test)) return;

    static float elapsed_time = 0;

    if (sig == SIGWINCH)
    {
        endwin();
        initscr();
        struct winsize ws;
        ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
        resizeterm(ws.ws_row, ws.ws_col);

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
    }
    else
    {
        struct timespec new_time;
        clock_gettime(CLOCK_REALTIME, &new_time);
        if (!timer_ispaused)
        {
            elapsed_time += (new_time.tv_nsec - last_time.tv_nsec) / 1000000000.f;
            elapsed_time += new_time.tv_sec - last_time.tv_sec;
        }
        last_time = new_time;

        print_at(1, LINES - 2, simple_filter, "%*.2fs", COLS - 4, elapsed_time);
    }

    refresh();

    atomic_flag_clear(&sigwinch_test);
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

    init_keyboard();

    clock_gettime(CLOCK_REALTIME, &last_time);

    atexit((void*)endwin);
    setup_signals();
    signal(SIGWINCH, sigwinch);

    initscr();
    noecho();
    cbreak();
    curs_set(0);

    setup_rendering_and_xml_data = true;

    sigwinch(SIGWINCH);

    while (true)
    {
        sigwinch(0);
        handle_input_and_timeout();
    }

    endwin();
}
