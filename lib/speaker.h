/**
 * @file speaker.h
 * @author Kudryashov Ilya (kudriashov.it@phystech.edu)
 * @brief Module for TTS functionality.
 * @version 0.1
 * @date 2022-11-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef SPEAKER_H
#define SPEAKER_H

const unsigned long long MAX_LINE_LENGTH = 4096;

#ifndef SILENT
#define say(format, ...) _say(format, __VA_ARGS__)
#else
#define say(format, ...) (0)
#endif

/**
 * @brief Say one string of text.
 * 
 * @param format format string, same as for printf
 * @param __va_args__ arguments, same as for printf
 */
void _say(const char* format, ...) __attribute__((format (printf, 1, 2)));

#endif