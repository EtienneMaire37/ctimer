#include "time_conversion.h"
#include <stdlib.h>

float time_to_seconds(const char* str)
{
    if (!str) abort();
    char* endptr;
    long first_part = strtol(str, &endptr, 10);
    if (!*endptr)
        return -1;
    str = endptr + 1;
    long second_part = strtol(str, &endptr, 10);
    if (!*endptr)
        return -2;
    str = endptr + 1;
    float third_part = strtof(str, &endptr);
    if (*endptr)
        return -3;
    return third_part + 60 * second_part + 3600 * first_part;
}
