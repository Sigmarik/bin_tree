#include "bin_tree.h"

#include <sys/stat.h>
#include <cstring>
#include <time.h>

#include "util/dbg/debug.h"
#include "file_helper.h"

#include "tree_config.h"

/**
 * @brief Destroy node of the tree with all of its children.
 * 
 * @param node node to destroy
 */
static void recursive_dtor(TreeNode* node);

/**
 * @brief Read single node from the stream.
 * 
 * @param node node to put the result in
 * @param file stream to read from
 * @param err_code variable to use as errno
 */
static void read_node(TreeNode* node, FILE* file, int* const err_code = NULL);

void TreeNode_ctor(TreeNode* node, char* value, bool free_value, TreeNode* parent, bool is_right, int* const err_code) {
    _LOG_FAIL_CHECK_(node,  "error", ERROR_REPORTS, return, err_code, EINVAL);

    node->value = value;
    node->free_value = free_value;
    if (parent) {
        _LOG_FAIL_CHECK_((is_right ? parent->right : parent->left) == NULL, 
                         "error", ERROR_REPORTS, return, err_code, EINVAL);
        (is_right ? parent->right : parent->left) = node;
        node->parent = parent;
    }
}

void TreeNode_dtor(TreeNode* node) {
    if (!node) return;

    if (node->parent) {
        if (node->parent->left  == node) node->parent->left  = NULL;
        if (node->parent->right == node) node->parent->right = NULL;
    }

    if (node->free_value) {
        free(node->value);
        node->value = NULL;
    }

    if (node->left)  node->left->parent  = NULL;
    if (node->right) node->right->parent = NULL;
}

void BinaryTree_ctor(BinaryTree* const tree, int* const err_code) {
    _LOG_FAIL_CHECK_(tree, "error", ERROR_REPORTS, return, err_code, EINVAL);
    
    tree->root = (TreeNode*) calloc(1, sizeof(*tree->root));
    _LOG_FAIL_CHECK_(tree->root, "error", ERROR_REPORTS, return, err_code, ENOMEM);

    TreeNode_ctor(tree->root, NULL, false, NULL, false, err_code);
}

void BinaryTree_dtor(BinaryTree* const tree) {
    recursive_dtor(tree->root);
    tree->root = NULL;
}

void BinaryTree_read(BinaryTree* const tree, FILE* file, int* const err_code) {
    _LOG_FAIL_CHECK_(tree, "error", ERROR_REPORTS, return, err_code, EINVAL);
    _LOG_FAIL_CHECK_(file, "error", ERROR_REPORTS, return, err_code, EINVAL);

    tree->root = (TreeNode*) calloc(1, sizeof(*tree->root));
    _LOG_FAIL_CHECK_(tree->root, "error", ERROR_REPORTS, return, err_code, ENOMEM);

    read_node(tree->root, file, err_code);
}

void TreeNode_graph_dump(const TreeNode* node, FILE* file) {
    if (!node || !file) return;
    fprintf(file, "\tV%p [label=\"%s\"]\n", node, node->value ? node->value : "NULL");

    if (node->parent) {
        fprintf(file, "\tV%p -> V%p [color=\"%s\"]\n", node->parent, node, 
                node == node->parent->left ? "darkgreen" : "darkred");
    }

    if (node->left) TreeNode_graph_dump(node->left, file);
    if (node->right) TreeNode_graph_dump(node->right, file);
}

static size_t PictCount = 0;

void _BinaryTree_dump_graph(const BinaryTree* const tree, unsigned int importance) {
    BinaryTree_status_t status = BinaryTree_status(tree);
    _log_printf(importance, "tree_dump", "\tTree at %p (status = %d):\n", tree, status);
    if (status) {
        for (size_t error_id = 0; error_id < TREE_REPORT_COUNT; ++error_id) {
            if (status & (1 << error_id)) {
                _log_printf(importance, "tree_dump", "\t%s\n", TREE_STATUS_DESCR[error_id]);
            }
        }
    }

    if (status & ~TREE_INV_CONNECTIONS) return;

    FILE* temp_file = fopen(TREE_TEMP_DOT_FNAME, "w");
    
    _LOG_FAIL_CHECK_(temp_file, "error", ERROR_REPORTS, return, NULL, 0);

    fputs("digraph G {\n", temp_file);
    fputs(  "\trankdir=TB\n"
            "\tlayout=dot\n"
            , temp_file);

    if (tree->root) TreeNode_graph_dump(tree->root, temp_file);

    fputc('}', temp_file);
    fclose(temp_file);

    if (system("mkdir -p " TREE_LOG_ASSET_FOLD_NAME)) return;

    time_t raw_time = 0;
    time(&raw_time);

    char pict_name[TREE_PICT_NAME_SIZE] = "";
    sprintf(pict_name, TREE_LOG_ASSET_FOLD_NAME "/pict%04ld_%ld.png", (long int)++PictCount, raw_time);

    char draw_request[TREE_DRAW_REQUEST_SIZE] = "";
    sprintf(draw_request, "dot -Tpng -o %s " TREE_TEMP_DOT_FNAME, pict_name);

    if (system(draw_request)) return;

    _log_printf(importance, "tree_dump",
                "\n<details><summary>Graph</summary><img src=\"%s\"></details>\n", pict_name);
}

TreeNode* BinaryTree_find(const BinaryTree* const tree, const char* word, int* const err_code) {
    _LOG_FAIL_CHECK_(!BinaryTree_status(tree), "error", ERROR_REPORTS, return NULL, err_code, EFAULT);
    _LOG_FAIL_CHECK_(word, "error", ERROR_REPORTS, return NULL, err_code, EFAULT);

    TreeNode* node = tree->root;
    TreeNode* prev = node->parent;

    do {
        TreeNode* prev_mem = prev;
        prev = node;

        if (!(node->left && node->right)) {

            if (strcmp(word, node->value) == 0) return node;
            else node = node->parent;

        } else if (prev_mem == node->parent) {
            node = node->left;
        } else if (prev_mem == node->left) {
            node = node->right;
        } else if (prev_mem == node->right) {
            node = node->parent;
        }
    } while (node != NULL);

    return NULL;
}

void BinaryTree_fill_path(const TreeNode* node, const TreeNode* *path, size_t* const out_length, 
                          const size_t max_length, int* const err_code) {
    _LOG_FAIL_CHECK_(node,       "error", ERROR_REPORTS, return, err_code, EFAULT);
    _LOG_FAIL_CHECK_(path,       "error", ERROR_REPORTS, return, err_code, EINVAL);
    _LOG_FAIL_CHECK_(out_length, "error", ERROR_REPORTS, return, err_code, EFAULT);

    for (; node && *out_length < max_length; node = node->parent, ++*out_length) {
        path[*out_length] = node;
    }

    for (size_t index = 0; 2 * index < *out_length; index++) {
        const TreeNode* swap_buffer = path[index];
        path[index] = path[*out_length - index - 1];
        path[*out_length - index - 1] = swap_buffer;
    }
}

void BinaryTree_write_content(const BinaryTree* tree, FILE* const file, int* const err_code) {
    _LOG_FAIL_CHECK_(!BinaryTree_status(tree), "error", ERROR_REPORTS, return, err_code, EINVAL);
    _LOG_FAIL_CHECK_(file, "error", ERROR_REPORTS, return, err_code, EINVAL);

    TreeNode_write_content(tree->root, file, 0, err_code);
}

void TreeNode_write_content(const TreeNode* node, FILE* const file, int shift, int* const err_code) {
    _LOG_FAIL_CHECK_(node, "error", ERROR_REPORTS, return, err_code, EINVAL);
    _LOG_FAIL_CHECK_(file, "error", ERROR_REPORTS, return, err_code, EINVAL);
    for (int index = 0; index < shift; index++) fputc('\t', file);
    fprintf(file, "{\"%s\"", node->value);
    if (node->left) {
        fprintf(file, ",\n");
        TreeNode_write_content(node->left,  file, shift + 1, err_code);
        fputc(',', file);
    }
    if (node->right) {
        fputc('\n', file);
        TreeNode_write_content(node->right, file, shift + 1, err_code);
        fputc('\n', file);
        for (int index = 0; index < shift; index++) fputc('\t', file);
    }
    fputc('}', file);
}

BinaryTree_status_t BinaryTree_status(const BinaryTree* tree) {
    if (tree == NULL) return TREE_NULL;
    if (tree->root == NULL) return TREE_NULL_ROOT;
    #ifndef NDEBUG
        return TreeNode_status(tree->root);
    #else
        return 0;
    #endif
}

BinaryTree_status_t TreeNode_status(const TreeNode* node) {
    _LOG_FAIL_CHECK_(node, "error", ERROR_REPORTS, return TREE_INV_CONNECTIONS, &errno, EFAULT);

    if (((bool)node->left) != ((bool)node->right)) return TREE_INV_CONNECTIONS;
    if (!node->left) return 0;
    if (node->left->parent != node) return TREE_INV_CONNECTIONS;
    if (node->right->parent != node) return TREE_INV_CONNECTIONS;
    return TreeNode_status(node->left) | TreeNode_status(node->right);
}

static void recursive_dtor(TreeNode* node) {
    if (node == NULL) return;
    if (node->free_value) free(node->value);
    if (node->left) recursive_dtor(node->left);
    if (node->right) recursive_dtor(node->right);
    free(node);
}

static void read_node(TreeNode* node, FILE* file, int* const err_code) {
    _LOG_FAIL_CHECK_(node, "error", ERROR_REPORTS, return, err_code, ENOENT);
    _LOG_FAIL_CHECK_(file, "error", ERROR_REPORTS, return, err_code, EINVAL);
    _LOG_FAIL_CHECK_(node->value == NULL, "error", ERROR_REPORTS, return, err_code, ENOENT);
    _LOG_FAIL_CHECK_(node->left == NULL,  "error", ERROR_REPORTS, return, err_code, ENOENT);
    _LOG_FAIL_CHECK_(node->right == NULL, "error", ERROR_REPORTS, return, err_code, ENOENT);

    skip_to_char(file, '"');

                                                       /* v One extra zero character to avoid overflow */
    char* temp_buffer = (char*) calloc(MAX_VALUE_LENGTH + 1, sizeof(*temp_buffer));
    _LOG_FAIL_CHECK_(temp_buffer, "error", ERROR_REPORTS, return, err_code, ENOMEM);

    int length = skip_to_char(file, '"', temp_buffer, MAX_VALUE_LENGTH);
    _LOG_FAIL_CHECK_(length >= 0, "error", ERROR_REPORTS, { free(temp_buffer); return; }, err_code, EINVAL);

    node->value = (char*) calloc((size_t)length + 1, sizeof(*node->value));
    _LOG_FAIL_CHECK_(node->value, "error", ERROR_REPORTS, return, err_code, ENOMEM);

    node->free_value = true;

    memcpy(node->value, temp_buffer, ((size_t)length + 1) * sizeof(*node->value));

    free(temp_buffer);

    exec_on_char(file, {
        case EOF:
        case '}': return;

        case '{': {
            TreeNode** target_ptr = &node->left;
            if (node->left) target_ptr = &node->right;

            _LOG_FAIL_CHECK_(*target_ptr == NULL, "error", ERROR_REPORTS, {
                log_printf(ERROR_REPORTS, "error", "Failed to read node from file. Too many children nodes were specified.");
                return;
            }, err_code, EINVAL);

            *target_ptr = (TreeNode*) calloc(1, sizeof(**target_ptr));
            _LOG_FAIL_CHECK_(*target_ptr, "error", ERROR_REPORTS, return, err_code, ENOMEM);

            (*target_ptr)->parent = node;

            read_node(*target_ptr, file, err_code);
        }

        default: break;
    });
}
