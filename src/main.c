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
#include "time_conversion.h"

#define YELLOW_TEXT     COLOR_PAIR(1)
#define BLUE_TEXT       COLOR_PAIR(2)
#define GREEN_TEXT      COLOR_PAIR(3)
#define BLUE_HIGHLIGHT  COLOR_PAIR(4)

bool setup_rendering_and_xml_data = false;
xml_tag_t* first_tag;
xml_tag_t *GameName, *CategoryName, *FirstSegment;

struct timespec last_time = {0};

atomic_flag sigwinch_test = ATOMIC_FLAG_INIT;

void sigwinch(int sig)
{
    if (!setup_rendering_and_xml_data) return;

    if (atomic_flag_test_and_set(&sigwinch_test)) return;

    if (sig == SIGWINCH || sig == 1)
    {
        if (sig == SIGWINCH)
        {
            endwin();
            initscr();
            struct winsize ws;
            ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
            resizeterm(ws.ws_row, ws.ws_col);
        }

        clear();

        attron(BLUE_TEXT);
        border(0, 0, 0, 0, 0, 0, 0, 0);
        attroff(BLUE_TEXT);
        attron(YELLOW_TEXT);
        print_at(2, 1, simple_filter, "%s - %s", GameName->data, CategoryName->data);
        attroff(YELLOW_TEXT);
        int seg = 0;
        xml_tag_t* Segment = FirstSegment;
        float cumul_pb = 0;
        int offset = current_segment - 3 + (LINES <= 9) + (LINES <= 10) + (LINES <= 11);
        if (offset < 0) offset = 0;
        if (LINES - 8 > total_segments) offset = 0;
        if (offset && total_segments < LINES - 8 + offset) offset = total_segments - LINES + 8;
        // TODO: Get the PB not best time
        while (Segment && seg < LINES - 8 + offset)
        {
            xml_tag_t* Name = xml_get_child(Segment, "Name");
            xml_tag_t* BestSegmentTime = xml_get_child(Segment, "BestSegmentTime");
            xml_tag_t* RealTime = xml_get_child(BestSegmentTime, "RealTime");
            if (RealTime)
                cumul_pb += time_to_seconds(RealTime->data);

            if (seg >= offset)
            {
                if (seg == current_segment) attron(BLUE_HIGHLIGHT);
                print_at(2, 3 + seg - offset, basic_filter, " * %-*s", COLS - 7, Name ? Name->data : "<Segment>");
                print_time_at(3 + seg - offset, cumul_pb);
                if (seg == current_segment) attroff(BLUE_HIGHLIGHT);
            }
            Segment = Segment->next;
            seg++;
        }

        if (Segment && LINES > 8 - offset)
        {
            while (Segment->next)
            {
                xml_tag_t* BestSegmentTime = xml_get_child(Segment, "BestSegmentTime");
                xml_tag_t* RealTime = xml_get_child(BestSegmentTime, "RealTime");
                if (RealTime)
                    cumul_pb += time_to_seconds(RealTime->data);
                Segment = Segment->next;
                seg++;
            }
            xml_tag_t* Name = xml_get_child(Segment, "Name");
            xml_tag_t* BestSegmentTime = xml_get_child(Segment, "BestSegmentTime");
            xml_tag_t* RealTime = xml_get_child(BestSegmentTime, "RealTime");
            if (RealTime)
                cumul_pb += time_to_seconds(RealTime->data);
            if (seg == current_segment) attron(BLUE_HIGHLIGHT);
            print_at(2, LINES - 4, basic_filter, " * %-*s", COLS - 7, Name ? Name->data : "<Segment>");
            print_time_at(LINES - 4, cumul_pb);
            if (seg == current_segment) attroff(BLUE_HIGHLIGHT);
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

        if (LINES > 3)
        {
            attron(GREEN_TEXT);
            print_time_at(LINES - 2, elapsed_time);
            attroff(GREEN_TEXT);
        }
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
    xml_tag_t* Segment = FirstSegment;
    total_segments = 0;
    while ((Segment = Segment->next))
        total_segments++;

    init_keyboard();

    clock_gettime(CLOCK_REALTIME, &last_time);

    atexit((void*)endwin);
    setup_signals();
    signal(SIGWINCH, sigwinch);

    initscr();
    noecho();
    cbreak();
    curs_set(0);
    start_color();
    init_pair(1, COLOR_YELLOW, COLOR_BLACK);
    init_pair(2, COLOR_BLUE, COLOR_BLACK);
    init_pair(3, COLOR_GREEN, COLOR_BLACK);
    init_pair(4, COLOR_WHITE, COLOR_BLUE);

    setup_rendering_and_xml_data = true;

    sigwinch(1);

    while (true)
    {
        sigwinch(0);
        handle_input_and_timeout();
    }

    endwin();
}
