#include "file_helper.h"

#include <sys/stat.h>

int skip_to_char(FILE* file, const char target, char* const buffer, size_t limit) {
    if (file == NULL) return -1;

    int count = 0;

    while (true) {
        char in_c = (char)fgetc(file);

        if (in_c == target) return count;
        if (in_c == EOF) return -1;

        if (buffer && (count < (int)limit || limit == 0)) buffer[count] = in_c;
        count++;
    }
}

void fclose_void(FILE** ptr) {
    if (ptr == NULL) return;
    if (*ptr) fclose(*ptr);
}

size_t get_file_size(int fd) {
    struct stat buffer;
    fstat(fd, &buffer);
    return (size_t)buffer.st_size;
}