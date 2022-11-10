/**
 * @file bin_tree.h
 * @author Kudryashov Ilya (kudriashov.it@phystech.edu)
 * @brief Binary tree data structure.
 * @version 0.1
 * @date 2022-11-01
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef BIN_TREE_H
#define BIN_TREE_H

#include <stdlib.h>
#include <stdio.h>

#include "tree_config.h"
#include "bin_tree_reports.h"

struct TreeNode {
    TreeNode* parent = NULL;
    char* value = NULL;
    TreeNode* left = NULL;
    TreeNode* right = NULL;
    bool free_value = false;
};

void TreeNode_ctor(TreeNode* node, char* value, bool free_value, TreeNode* parent, bool is_right, int* const err_code = NULL);
void TreeNode_dtor(TreeNode* node);

struct BinaryTree {
    TreeNode* root = NULL;
};

void BinaryTree_ctor(BinaryTree* const tree, int* const err_code = NULL);
void BinaryTree_dtor(BinaryTree* const tree);

/**
 * @brief Create binary tree from given data base.
 * 
 * @param tree 
 * @param file 
 */
void BinaryTree_read(BinaryTree* const tree, FILE* file, int* const err_code = NULL);

/**
 * @brief Dump subtree into dot file.
 * 
 * @param node node to dump
 * @param file destination file
 */
void TreeNode_graph_dump(const TreeNode* node, FILE* file);

/**
 * @brief Dump the list into logs.
 * 
 * @param list
 * @param importance message importance
 */
#define BinaryTree_dump(tree, importance) do {                          \
    log_printf(importance, TREE_DUMP_TAG, "Called list dumping.\n");    \
    _BinaryTree_dump_graph(tree, importance);                           \
} while (0)

/**
 * @brief Put a picture of the tree into logs.
 * 
 * @param tree 
 * @param importance 
 */
void _BinaryTree_dump_graph(const BinaryTree* const tree, unsigned int importance);

/**
 * @brief Find the node with specified value in the tree.
 * 
 * @param tree tree to search in
 * @param word searched node value
 * @param err_code variable to use as errno
 * @return 
 */
TreeNode* BinaryTree_find(const BinaryTree* const tree, const char* word, int* const err_code = NULL);

/**
 * @brief Find the path to the node from the root of the tree.
 * 
 * @param node vertex to find the path to
 * @param path array to write the path to
 * @param out_length variable to put length of the path to
 * @param max_length maximal length of the path
 */
void BinaryTree_fill_path(const TreeNode* node, const TreeNode* *path, size_t* const out_length, 
                          const size_t max_length, int* const err_code = NULL);

/**
 * @brief Write tree content to the file.
 * 
 * @param tree tree to write to the file
 * @param file write destination
 * @param err_code variable to use as errno
 */
void BinaryTree_write_content(const BinaryTree* tree, FILE* const file, int* const err_code = NULL);

/**
 * @brief Write node content to the file.
 * 
 * @param node tree node to write to the file
 * @param file write destination
 * @param shift depth of the node
 * @param err_code variable to use as errno
 */
void TreeNode_write_content(const TreeNode* node, FILE* const file, int shift, int* const err_code = NULL);

/**
 * @brief Get status of the tree.
 * 
 * @param tree 
 * @return (BinaryTree_status_t) binary tree status (0 = OK)
 */
BinaryTree_status_t BinaryTree_status(const BinaryTree* tree);

/**
 * @brief Get status of the connections of the node all all of its subnodes.
 * 
 * @param node
 * @return (BinaryTree_status_t) node connection status (0 = OK)
 */
BinaryTree_status_t TreeNode_status(const TreeNode* node);

#endif
