#include "main_utils.h"

#include <stdlib.h>
#include <stdarg.h>

/**
 * @brief Print one parameter of the object in the form of "is (not) an object(, )"
 * 
 * @param node argument to print
 * @param next_node nex node in definition path
 * @param is_last is the curent node the last one in the list
 */
static void say_argument(const TreeNode* node, const TreeNode* next_node, bool is_last);

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
    printf("Guesser game by Ilya Kudryashov.\n");
    printf("Program uses binary tree to guess things.\n");
    printf("Build from\n%s %s\n", __DATE__, __TIME__);
    log_printf(ABSOLUTE_IMPORTANCE, "build info", "Build from %s %s.\n", __DATE__, __TIME__);
}

const char* get_input_file_name(const int argc, const char** argv) {
    const char* file_name = NULL;

    for (int argument_id = 1; argument_id < argc; ++argument_id) {
        if (*argv[argument_id] == '-') continue;
        file_name = argv[argument_id];
        break;
    }

    return file_name;
}

const char* get_output_file_name(const int argc, const char** argv) {
    const char* file_name = NULL;

    bool enc_first_name = false;
    for (int argument_id = 1; argument_id < argc; ++argument_id) {
        if (*argv[argument_id] == '-') continue;
        file_name = argv[argument_id];
        if (enc_first_name) return file_name;
        else enc_first_name = true;
    }

    return NULL;
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
        fgets(word, MAX_INPUT_LENGTH - 1, stdin);
        define(tree, word, err_code);
        break;
    }
    case 'P': {
        log_printf(ABSOLUTE_IMPORTANCE, "dump_info", "Called dump on user request.\n");
        BinaryTree_dump(tree, ABSOLUTE_IMPORTANCE);
        break;
    }
    case 'C': {
        printf("What do you want me to compare with something?\n>>> ");

        char word_a[MAX_INPUT_LENGTH] = "";
        fgets(word_a, MAX_INPUT_LENGTH - 1, stdin);
        word_a[strlen(word_a) - 1] = '\0';

        printf("What do you want me to compare %s to?\n>>> ", word_a);

        char word_b[MAX_INPUT_LENGTH] = "";
        fgets(word_b, MAX_INPUT_LENGTH - 1, stdin);
        word_b[strlen(word_b) - 1] = '\0';

        compare(tree, word_a, word_b, err_code);
        break;
    }
    default: {
        printf("Incorrect command, enter command from the list.\n");
        break;
    }
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

    const TreeNode* node = BinaryTree_find(tree, word, err_code);
    if (!node) {
        printf("Word was not found!\n");
    } else {
        printf("It ");

        const TreeNode* chain[MAX_TREE_DEPTH] = {};
        size_t depth = 0;

        BinaryTree_fill_path(node, chain, &depth, MAX_TREE_DEPTH, err_code);

        --depth; // <- Account for the answer node at the end of each path.

        for (size_t index = 0; index < depth; ++index) {
            say_argument(chain[index], chain[index + 1], index == depth - 1);
        }

        printf(".\n");
    }
}

void compare(BinaryTree* tree, const char* word_a, const char* word_b, int* const err_code) {
    _LOG_FAIL_CHECK_(tree,   "error", ERROR_REPORTS, return, err_code, EFAULT);
    _LOG_FAIL_CHECK_(word_a, "error", ERROR_REPORTS, return, err_code, EFAULT);
    _LOG_FAIL_CHECK_(word_b, "error", ERROR_REPORTS, return, err_code, EFAULT);

    const TreeNode* node_a = BinaryTree_find(tree, word_a, err_code);
    const TreeNode* node_b = BinaryTree_find(tree, word_b, err_code);

    if (node_a == NULL || node_b == NULL) {
        puts("One of the words was not found.");

        return;

    }

    if (node_a == node_b) {
        puts("They are the same objects...");

        return;

    }

    // TODO: Can anyone see what is the problem here?
    // TODO: Can anyone see what is the problem here?
    const TreeNode* path_a[MAX_TREE_DEPTH] = {};
    const TreeNode* path_b[MAX_TREE_DEPTH] = {};
    size_t depth_a = 0;
    size_t depth_b = 0;
    BinaryTree_fill_path(node_a, path_a, &depth_a, MAX_TREE_DEPTH, err_code);
    BinaryTree_fill_path(node_b, path_b, &depth_b, MAX_TREE_DEPTH, err_code);

    size_t prefix_length = 0;

    for (size_t index = 1; index < depth_a && index < depth_b; ++index) {
        if (path_a[index] == path_b[index]) continue;
        prefix_length = index - 1;
        break;
    }
    
    if (prefix_length == 0) {
        puts("These objects have nothing in common, as");
    } else {
        printf("These objects are similar to each other as they both can be described as \"");
        for (size_t index = 0; index < prefix_length; ++index) {
            say_argument(path_a[index], path_a[index + 1], index == prefix_length - 1);
        }
        puts("\", while");
    }

    --depth_a;
    --depth_b;

    printf("object %s ", word_a);
    for (size_t index = prefix_length; index < depth_a; ++index) {
        say_argument(path_a[index], path_a[index + 1], index == depth_a - 1);
    }

    printf(", and\nobject %s ", word_b);
    for (size_t index = prefix_length; index < depth_b; ++index) {
        say_argument(path_b[index], path_b[index + 1], index == depth_b - 1);
    }
    puts(".");
}

static void say_argument(const TreeNode* node, const TreeNode* next_node, bool is_last) {
    printf("is ");
    if (node->right == next_node) printf("not ");
    printf("%s", node->value);
    if (!is_last) printf(", ");
}
