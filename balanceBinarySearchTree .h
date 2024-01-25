#ifndef __BALANCEBINARY_SEARCH_TREE_H_
#define __BALANCEBINARY_SEARCH_TREE_H_

#include "doubleLinkListQueue.h"
//#define ELEMENTTYPE int


typedef struct AVLTreeNode
{
    ELEMENTTYPE data;
    int height;
    struct AVLTreeNode *left;        /* 左子树 */
    struct AVLTreeNode *right;       /* 右子树 */
    #if 1
    struct AVLTreeNode *parent;      /* 父节点 */
    #endif

}AVLTreeNode;

typedef struct BalanceBinarySearchTree
{
    /* 根节点 */
    AVLTreeNode *root;
    /* 树的结点个数 */
    int size;
    /* 树的高度 */
    int height;
    /* 钩子函数比较器放在结构体 */
    int (*compareFunc)(ELEMENTTYPE val1, ELEMENTTYPE val2);

    /* 钩子函数 包装器(实现自定义打印函数接口) */
    int (*printFunc)(ELEMENTTYPE val);
    /* 把队列的属性放到树里面 */
    //doubleLinkListQueue * pQueue;

}BalanceBinarySearchTree;

/* 二叉搜素树的初始化 */
int balanceBinarySearchTreeInit(BalanceBinarySearchTree **pBstree, int(*compareFunc)(ELEMENTTYPE val1, ELEMENTTYPE val2), int(*printFunc)(ELEMENTTYPE val));

/* 二叉搜索树的插入 */
int balanceBinarySearchTreeInsert(BalanceBinarySearchTree *pBstree, ELEMENTTYPE val);

/* 二叉搜索时是否包含指定元素 */
int balanceBinarySearchTreeIsContainAppointVal(BalanceBinarySearchTree *pBstree, ELEMENTTYPE val);

/* 二叉搜索树的前序遍历 */
int balanceBinarySearchTreePreOrderTravel(BalanceBinarySearchTree *pBstree);

/* 二叉搜索树的中序遍历 */
int balanceBinarySearchTreeInOrderTravel(BalanceBinarySearchTree *pBstree);

/* 二叉搜索树的后序遍历 */
int balanceBinarySearchTreePosOrderTravel(BalanceBinarySearchTree *pBstree);

/* 二叉搜索树的层序遍历 */
int balanceBinarySearchTreeLevelOrderTravel(BalanceBinarySearchTree *pBstree);

/* 获取二叉搜索树的结点个数 */
int balanceBinarySearchTreeGetNodeSize(BalanceBinarySearchTree *pBstree, int *pSize);

/* 获取二叉搜树的高度 */
int balanceBinarySearchTreeGetHeight(BalanceBinarySearchTree *pBstree, int *pHeight);

/* 二叉搜索树的删除 */
int balanceBinarySearchTreeDelete(BalanceBinarySearchTree *pBstree, ELEMENTTYPE val);

/* 判断树是否是完全二叉树 */
int balanceBinarySearchTreeIsComplete(BalanceBinarySearchTree *pBstree);

/* 二叉搜索树的销毁*/
int balanceBinarySearchTreeDestory(BalanceBinarySearchTree * pBstree);



#endif