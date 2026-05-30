#pragma once

#include <signal.h>
#include <ncurses.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

static inline void sigwinch()
{
    refresh();
}

static inline void exit_from_signal(int sig)
{
    exit(-sig);
}

static inline void setup_signals()
{
    for (int i = 0; i < SIGRTMIN; i++)
        signal(i, exit_from_signal);
    signal(SIGWINCH, sigwinch);
}
