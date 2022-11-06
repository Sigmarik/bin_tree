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
void TreeNode_graph_dump(TreeNode* node, FILE* file);

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
void _BinaryTree_dump_graph(BinaryTree* const tree, unsigned int importance);

/**
 * @brief Find the node with specified value in the tree.
 * 
 * @param tree tree to search in
 * @param word searched node value
 * @param err_code variable to use as errno
 * @return 
 */
TreeNode* BinaryTree_find(BinaryTree* const tree, const char* word, int* const err_code = NULL);

#endif
