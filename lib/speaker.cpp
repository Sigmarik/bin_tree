#define SPEAKER_CPP
#include "speaker.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#include "util/dbg/debug.h"

static bool speaker_mute = false;

void _say(const char* format, ...) {
    if (speaker_mute) return;

    va_list args;
    va_start(args, format);

                                /*   v- length of the TTS module name */
    char request[MAX_PHRASE_LENGTH + 9] = "espeak \"";

    vsprintf(request + strlen(request), format, args);
    log_printf(STATUS_REPORTS, "status", "%s", request);
    request[strlen(request)] = '"';

    if (system(request) < 0) {
        log_printf(WARNINGS, "warning", "Failed to access espeak module, the speaker was muted.");
        speaker_set_mute(true);
    }

    va_end(args);
}

void speaker_set_mute(bool new_mute) {
    speaker_mute = new_mute;
}

bool speaker_get_mute() {
    return speaker_mute;
}