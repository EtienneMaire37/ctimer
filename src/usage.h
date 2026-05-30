#pragma once

#include <stdio.h>

static inline void print_usage()
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "ctimer [FILE]\n");
}
