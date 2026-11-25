#include "myalloc.h"
#include <stdio.h>

typedef struct TreeNode {
    int key;
    struct TreeNode *left;
    struct TreeNode *right;
} TreeNode;

static TreeNode *insert(TreeNode *root, int key) {
    if (!root) {
        TreeNode *n = my_calloc(1, sizeof(TreeNode));
        n->key = key;
        return n;
    }
    if (key < root->key) {
        root->left = insert(root->left, key);
    } else if (key > root->key) {
        root->right = insert(root->right, key);
    }
    return root;
}

static void inorder_and_free(TreeNode *root, int *count, int *weighted_sum) {
    if (!root) return;
    inorder_and_free(root->left, count, weighted_sum);
    (*count)++;
    *weighted_sum += (*count) * root->key;
    TreeNode *right = root->right;
    my_free(root);
    inorder_and_free(right, count, weighted_sum);
}

int main(void) {
    my_set_policy(WORST_FIT);
    my_set_delay(8);

    TreeNode *root = NULL;
    /* Insert a permutation of 1..20 into the BST */
    int keys[20] = {11, 5, 16, 3, 8, 14, 19, 1, 4, 7, 10, 12, 15, 18, 20, 2, 6, 9, 13, 17};
    for (int i = 0; i < 20; i++) {
        root = insert(root, keys[i]);
    }

    int count = 0;
    int weighted_sum = 0;
    inorder_and_free(root, &count, &weighted_sum);
    /* In-order traversal visits 1..20 in sorted order, so i-th visited has key = i.
       weighted_sum = sum_{i=1..20} i^2 = 20 * 21 * 41 / 6 = 2870. */
    printf("count=%d weighted_sum=%d\n", count, weighted_sum);
    my_leak_report();
    return 0;
}
