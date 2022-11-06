/**
 * @file main.c
 * @author Ilya Kudryashov (kudriashov.it@phystech.edu)
 * @brief Listworks library showcase.
 * @version 0.1
 * @date 2022-08-26
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <ctype.h>

#include "lib/util/dbg/debug.h"
#include "lib/util/argparser.h"
#include "lib/alloc_tracker/alloc_tracker.h"
#include "utils/main_utils.h"
#include "lib/file_helper.h"

#include "utils/config.h"

#include "lib/bin_tree.h"

#define MAIN

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

int main(const int argc, const char** argv) {
    atexit(log_end_program);

    unsigned int log_threshold = STATUS_REPORTS + 1;

    ActionTag line_tags[] = {
        #include "cmd_flags/main_flags.h"
    };
    const int number_of_tags = sizeof(line_tags) / sizeof(*line_tags);

    parse_args(argc, argv, number_of_tags, line_tags);
    log_init("program_log.html", log_threshold, &errno);
    print_label();

    const char* f_name = DEFAULT_DB_NAME;
    const char* suggested_name = get_input_file_name(argc, argv);
    if (suggested_name) f_name = suggested_name;

    FILE* source_db = fopen(f_name, "r");
    _LOG_FAIL_CHECK_(source_db, "error", ERROR_REPORTS, {
        log_printf(ERROR_REPORTS, "error", "Failed to open file %s.\n", f_name);

        return_clean(EXIT_FAILURE);

    }, &errno, ENOENT);
    track_allocation(&source_db, (dtor_t*)fclose_void);

    setvbuf(source_db, NULL, _IOFBF, get_file_size(fileno(source_db)));

    BinaryTree decision_tree = {};

    BinaryTree_read(&decision_tree, source_db, &errno);

    BinaryTree_dump(&decision_tree, ABSOLUTE_IMPORTANCE);

    bool running = true;
    while (running) {
        char command = '\0';

        printf("Command (Q - quit, G - guess, D - definition, C - compare)\n>>> ");
        scanf(" %c", &command);
        command = toupper(command);

        log_printf(STATUS_REPORTS, "status", "Encountered command %c.\n", command);

        if (command == 'Q') running = false;
        else execute_command(command, &decision_tree);
    }

    return_clean(errno == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}

void execute_command(char cmd, BinaryTree* tree, int* const err_code) {
    switch(cmd) {
    case 'G': {
        guess(tree, err_code);
        break;
    }
    case 'D': {
        printf("Which word do you want me to give definition of?\n>>> ");
        char word[MAX_INPUT_LENGTH] = "";
        scanf("%s", &word);
        define(tree, word, err_code);
    }
    default:
        printf("Incorrect command, enter command from the list.\n");
        break;
    }
}

void guess(BinaryTree* tree, int* const err_code) {
    _LOG_FAIL_CHECK_(tree, "error", ERROR_REPORTS, return, err_code, EFAULT);

    TreeNode* node = tree->root;

    while (node->left && node->right) {
        printf("Is it %s? (yes/no)\n>>> ", node->value);
        yn_branch({ node = node->left; }, { node = node->right; });
    }

    printf("It must be %s. Is it? (yes/no)\n>>> ", node->value);
    yn_branch({ puts("Yay!\n"); }, {
        char new_name[MAX_INPUT_LENGTH] = "";

        printf("What is it, then?\n>>> ");

        fgets(new_name, MAX_INPUT_LENGTH, stdin);

        char* value_buffer = (char*) calloc(strnlen(new_name, MAX_INPUT_LENGTH) + 1, sizeof(*value_buffer));
        _LOG_FAIL_CHECK_(value_buffer, "error", ERROR_REPORTS, return, err_code, ENOMEM);
        memcpy(value_buffer, new_name, strnlen(new_name, MAX_INPUT_LENGTH) - 1);

        TreeNode* alpha_node = (TreeNode*) calloc(1, sizeof(*alpha_node));
        TreeNode_ctor(alpha_node, value_buffer, true, node, false, err_code);

        TreeNode* beta_node = (TreeNode*) calloc(1, sizeof(*beta_node));
        TreeNode_ctor(beta_node, node->value, node->free_value, node, true, err_code);

        printf("What is %s that %s is not?\nIt is ", value_buffer, node->value);

        char new_question[MAX_INPUT_LENGTH] = "";
        fgets(new_question, MAX_INPUT_LENGTH, stdin);

        char* separator = (char*) calloc(strnlen(new_question, MAX_INPUT_LENGTH) + 1, sizeof(*separator));
        _LOG_FAIL_CHECK_(separator, "error", ERROR_REPORTS, return, err_code, ENOMEM);
        memcpy(separator, new_question, strnlen(new_question, MAX_INPUT_LENGTH) - 1);

        node->value = separator;
        node->free_value = true;
    });
}

void define(BinaryTree* tree, const char* word, int* const err_code) {
    _LOG_FAIL_CHECK_(tree, "error", ERROR_REPORTS, return, err_code, EFAULT);
    _LOG_FAIL_CHECK_(word, "error", ERROR_REPORTS, return, err_code, EFAULT);

    TreeNode* node = BinaryTree_find(tree, word, err_code);
    if (!node) {
        printf("Word was not found!\n");
    } else {
        puts("It ");

        TreeNode* chain[MAX_TREE_DEPTH] = {};
        int depth = 0;
        do {
            chain[depth] = node;
            node = node->parent;
            ++depth;
        } while(node);
        for (int index = depth; index > 0; --index) {
            puts("is ");
            if (chain[depth] == chain[depth - 1]->left) puts("not ");
            puts(chain[index]->value);
        }
        puts(".\n");
    }
}

void compare(BinaryTree* tree, const char* word_a, const char* word_b, int* const err_code) {
    _LOG_FAIL_CHECK_(tree,   "error", ERROR_REPORTS, return, err_code, EFAULT);
    _LOG_FAIL_CHECK_(word_a, "error", ERROR_REPORTS, return, err_code, EFAULT);
    _LOG_FAIL_CHECK_(word_b, "error", ERROR_REPORTS, return, err_code, EFAULT);
}