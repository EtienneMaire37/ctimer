#pragma once

#include <signal.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <fcntl.h>
#include <sys/select.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

#include "timer.h"

extern int keyboard_fd;

extern char ____event_buffer[32];
#define event_(n) ({ snprintf(____event_buffer, sizeof(____event_buffer) - 1, "/dev/input/event%u", (n)); ____event_buffer; })

void sigwinch(int);

static inline bool is_keyboard(int fd)
{
    unsigned long key_bitmask[EV_MAX / 8 + 1] = {0};

    return ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(key_bitmask)), key_bitmask) >= 0;
}

static inline void init_keyboard()
{
    // TODO: Make this portable
    int i = 0;
    while ((keyboard_fd = open(event_(i++), O_RDONLY | O_NONBLOCK)) != -1)
    {
        if (is_keyboard(keyboard_fd)) break;
        close(keyboard_fd);
    }
    if (keyboard_fd == -1)
    {
        char buffer[32];
        snprintf(buffer, sizeof(buffer) - 1, "ctimer: `%s`", event_(i - 1));
        perror(buffer);
        abort();
    }
}

static inline void handle_input_and_timeout()
{
    // * 10ms
    struct timespec timeout = { .tv_sec = 0, .tv_nsec = 10000000 };
    fd_set rd_set;
    FD_ZERO(&rd_set);
    FD_SET(keyboard_fd, &rd_set);
    pselect(keyboard_fd + 1, &rd_set, NULL, NULL, &timeout, NULL);
    if (FD_ISSET(keyboard_fd, &rd_set))
    {
        struct input_event event;
        if (read(keyboard_fd, &event, sizeof(event)))
        {
            if (event.type == EV_KEY)
            {
                if (event.value == 1)
                {
                    switch (event.code)
                    {
                    case KEY_F1:
                    split:
                        if (timer_ispaused)
                        {
                            timer_ispaused = false;
                            if (current_segment == -1)
                                current_segment = 0;
                        }
                        else
                            current_segment++;
                        if (current_segment >= total_segments)
                            goto reset;
                        sigwinch(1);
                        break;
                    case KEY_F2:
                    reset:
                        timer_ispaused = true;
                        elapsed_time = 0;
                        current_segment = -1;
                        sigwinch(1);
                        break;
                    case KEY_F3:
                        if (current_segment == -1)
                            goto split;
                        timer_ispaused ^= true;
                        break;
                    default:
                        ;
                    }
                }
            }
        }
    }
}
