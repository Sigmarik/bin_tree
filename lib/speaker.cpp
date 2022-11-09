#include "speaker.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

void _say(const char* format, ...) {
    va_list args;
    va_start(args, format);

    char request[MAX_LINE_LENGTH] = "espeak \"";

    sprintf(request + strlen(request), format, args);
    request[strlen(request)] = '"';

    system(request);

    va_end(args);
}