#include "main_utils.h"

#include <stdlib.h>
#include <stdarg.h>

void** bundle(size_t count, ...) {
    va_list args;
    va_start(args, count);

    void** array = (void**) calloc(count, sizeof(*array));
    track_allocation(array, free);

    for (size_t index = 0; index < count; index++) {
        array[index] = va_arg(args, void*);
    }

    va_end(args);
    return array;
}

void MemorySegment_ctor(MemorySegment* segment) {
    segment->content = (int*) calloc(segment->size, sizeof(*segment->content));
}

void MemorySegment_dtor(MemorySegment* segment) {
    free(segment->content);
    segment->content = NULL;
    segment->size = 0;
}

void _MemorySegment_dump(MemorySegment* segment, unsigned int importance) {
    for (size_t id = 0; id < segment->size; ++id) {
        _log_printf(importance, "dump", "[%6lld] = %d\n", (long long) id, segment->content[id]);
    }
}

int clamp(const int value, const int left, const int right) {
    if (value < left) return left;
    if (value > right) return right;
    return value;
}

// Amazing, do not change anything!
// Completed the owl, sorry.
void print_owl(const int argc, void** argv, const char* argument) {
    SILENCE_UNUSED(argc); SILENCE_UNUSED(argv); SILENCE_UNUSED(argument);
    printf("-Owl argument detected, dropping emergency supply of owls.\n");
    for (int index = 0; index < NUMBER_OF_OWLS; index++) {
        puts(R"(    A_,,,_A    )");
        puts(R"(   ((O)V(O))   )");
        puts(R"(  ("\"|"|"/")  )");
        puts(R"(   \"|"|"|"/   )");
        puts(R"(     "| |"     )");
        puts(R"(      ^ ^      )");
    }
}

void print_label() {
    printf("Listworks lib showcase by Ilya Kudryashov.\n");
    printf("Program implements TUI for the list.\n");
    printf("Build from\n%s %s\n", __DATE__, __TIME__);
    log_printf(ABSOLUTE_IMPORTANCE, "build info", "Build from %s %s.\n", __DATE__, __TIME__);
}
