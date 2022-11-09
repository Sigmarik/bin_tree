/**
 * @file common.h
 * @author Kudryashov Ilya (kudriashov.it@phystech.edu)
 * @brief Utility features useful in all programs.
 * @version 0.1
 * @date 2022-10-22
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef COMMON_UTILS_H
#define COMMON_UTILS_H

#include <cstring>
#include <ctype.h>

#include "config.h"
#include "lib/alloc_tracker/alloc_tracker.h"
#include "lib/bin_tree.h"
#include "lib/file_helper.h"

//* We need this function as it is impossible to create static constant lists from local functions.
//* So... What it does is it creates a dynamic array and tracks it for later deletion.
/**
 * @brief Create list of void* pointers.
 * 
 * @param count number of elements
 * @param ... elements
 * @return array of elements
 */
void** bundle(size_t count, ...);

/**
 * @brief Array with stored size.
 * 
 */
struct MemorySegment {
    int* content = NULL;
    size_t size = 1024;
};

void MemorySegment_ctor(MemorySegment* segment);
void MemorySegment_dtor(MemorySegment* segment);

#define MemorySegment_dump(segment, importance) do { \
    log_printf(importance, "dump", "Memory segment at %p. Size: %ld Content: %p\n", segment, (segment)->size, (segment)->content); \
    _MemorySegment_dump(segment, importance); \
} while (0)

void _MemorySegment_dump(MemorySegment* segment, unsigned int importance);

/**
 * @brief Limit specified value between left and right border.
 * 
 * @param value variable to check
 * @param left left border
 * @param right right border
 * @return 
 */
int clamp(const int value, const int left, const int right);

/**
 * @brief Print a bunch of owls.
 * 
 * @param argc unimportant
 * @param argv unimportant
 * @param argument unimportant
 */
void print_owl(const int argc, void** argv, const char* argument);

/**
 * @brief Disable all voice lines.
 * 
 * @param argc unimportant
 * @param argv unimportant
 * @param argument unimportant
 */
void mute_speaker(const int argc, void** argv, const char* argument);

/**
 * @brief Print program label and build date/time to console and log.
 * 
 */
void print_label();

/**
 * @brief Get the input file name from the list of command line arguments.
 * 
 * @param argc argument count
 * @param argv argument values
 * @return const char* 
 */
const char* get_input_file_name(const int argc, const char** argv);

/**
 * @brief Get the output file name from the list of command line arguments.
 * 
 * @param argc argument count
 * @param argv argument values
 * @return const char* 
 */
const char* get_output_file_name(const int argc, const char** argv);

/**
 * @brief Read user input and do actions depending on if user entered yes or no.
 * 
 * @param action_yes code to execute on YES
 * @param action_no code to execute on NO
 */
#define yn_branch(action_yes, action_no) do {   \
    char __answer = '\0';                       \
    scanf(" %c", &__answer);                    \
    __answer = (char)tolower(__answer);         \
                                                \
    while (getc(stdin) != '\n');                \
                                                \
    if (__answer == 'y') { action_yes; break; } \
    if (__answer == 'n') { action_no;  break; } \
    printf("yes/no expected, try again.\n>>> ");\
} while(true)

/**
 * @brief Execute user command.
 * 
 * @param cmd command
 * @param tree decision tree
 * @param err_code variable to use as errno
 */
void execute_command(char cmd, BinaryTree* tree, int* const err_code = NULL);

/**
 * @brief Guess the word using user input.
 * 
 * @param tree tree to guess the word in
 * @param err_code variable to use as errno
 */
void guess(BinaryTree* tree, int* const err_code = NULL);

/**
 * @brief Give definition of the word.
 * 
 * @param tree tree to search in
 * @param word word to define
 * @param err_code variable to use as errno
 */
void define(BinaryTree* tree, const char* word, int* const err_code = NULL);

/**
 * @brief Compare definitions of two words.
 * 
 * @param tree tree to search in
 * @param word_a first word
 * @param word_b second word
 * @param err_code variable to use as errno
 */
void compare(BinaryTree* tree, const char* word_a, const char* word_b, int* const err_code = NULL);

#endif