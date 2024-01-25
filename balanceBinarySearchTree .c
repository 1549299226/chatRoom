#include "balanceBinarySearchTree .h"
#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include<math.h>
/* 状态码*/
enum STATUS_CODE
{
    ON_SUCCESS,
    NULL_PTR,
    MALLOC_ERROR,
    INVALID_ACCESS,
};

#define true 1
#define false 0


/* 静态函数前置声明 */
static int compareFunc(ELEMENTTYPE val1, ELEMENTTYPE val2);

/* 创建结点 */
static AVLTreeNode * createAVLTreeNewNode(ELEMENTTYPE val, AVLTreeNode *parent);

/* 根据指定的值获取二叉搜索树的结点 */
static AVLTreeNode * baseAppointValGetAVLTreeNode(BalanceBinarySearchTree *pBstree, ELEMENTTYPE val);

/* 判断二叉搜索树为2 */
static int balanceBinarySearchTreeNodeHasTwoChildren(AVLTreeNode *node);
/* 判断二叉搜索树度为1 */
static int balanceBinarySearchTreeNodeHasOneChild(AVLTreeNode *node);
/* 判断二叉搜索树度为0 */
static int balanceBinarySearchTreeNodeIsLeaf(AVLTreeNode *node);

/* 二叉搜索树的前序遍历 */
static int preOrderTravel(BalanceBinarySearchTree *pBstree, AVLTreeNode *node);
/* 二叉搜索树的中序遍历 */
static int inOrderTravel(BalanceBinarySearchTree *pBstree, AVLTreeNode *node);
/* 二叉搜索树的后序遍历 */
static int postOrderTravel(BalanceBinarySearchTree *pBstree, AVLTreeNode *node);

/* 获取当前结点的前驱结点 */
static AVLTreeNode * bstreeNodePreDecessor(AVLTreeNode * node);
/* 获取当前结点的后继结点 */
static AVLTreeNode * bstreeNodeSuccessor(AVLTreeNode * node);
/* 添加结点后的操作 */
static int insertNodeAfter(BalanceBinarySearchTree *pBstree, AVLTreeNode * node);
/* 计算结点的平衡因子 */
static int AVLTreeNodeBalanceFactor(AVLTreeNode *node);
/* 判断结果是否平衡*/
static int AVLTreeNodeIsBalanced(AVLTreeNode *node);
/* 二叉搜索树删除指定的节点 */
static int balanceBinarySearchTreeDelNode(BalanceBinarySearchTree *pBstree, AVLTreeNode *node);
/* 更新结点的高度 */
static int AVLTreeNodeUpdateHeight(AVLTreeNode *node);
/* 比较两个整数的最大值 */
static int tmpMax(int val1, int val2);
/* AVL树结点调整平衡 */
static int AVLTreeNodeAdjustBalance(BalanceBinarySearchTree *pBstree, AVLTreeNode * node);
/* 获取AVL结点较高的子结点 */
static AVLTreeNode * AVLTreeNodeGetChildTaller(AVLTreeNode * node);
/* 当前结点是父结点的左子树 */
static int AVLTreeCurrentNodeIsLeft(AVLTreeNode * node);
/* 当前结点是父结点的右子树 */
static int AVLTreeCurrentNodeIsRight(AVLTreeNode * node);
/* 左旋 */
static int AVLTreeCurrentNodeRatoteLeft(BalanceBinarySearchTree *pBstree, AVLTreeNode * node);
/* 右旋 */
static int AVLTreeCurrentNodeRatoteRight(BalanceBinarySearchTree *pBstree, AVLTreeNode * grand);
/* 旋转公共代码部分 */
static int AVLTreeNodeRotate(BalanceBinarySearchTree  *pBstree, AVLTreeNode * grand, AVLTreeNode *parent, AVLTreeNode * child);
/* 删除结点之后要做的事情 */
static int removeNodeAfter(BalanceBinarySearchTree *pBstree, AVLTreeNode * node);


/* 二叉搜索树的初始化 */
int balanceBinarySearchTreeInit(BalanceBinarySearchTree **pBstree, int(*compareFunc)(ELEMENTTYPE val1, ELEMENTTYPE val2), int(*printFunc)(ELEMENTTYPE val))
{
    int ret = 0;
    BalanceBinarySearchTree * bstree = (BalanceBinarySearchTree *)malloc(sizeof(BalanceBinarySearchTree) * 1);
    if(bstree == NULL)
    {
        return MALLOC_ERROR;
    }
    /* 清除脏数据 */
    memset(bstree, 0, sizeof(BalanceBinarySearchTree) * 1);
    /* 初始化树 */
    {
        bstree->root = NULL;
        bstree->size = 0;
        /* 钩子函数赋值 */
        bstree->compareFunc = compareFunc;
        /* 钩子函数包装器 */
        bstree->printFunc = printFunc;
    }
#if 0    
    /* 分配根节点 */
    bstree->root = (AVLTreeNode *)malloc(sizeof(AVLTreeNode) * 1);
    if(bstree->root == NULL)
    {
        return MALLOC_ERROR;
    }
    /* 清楚脏数据 */
    memset(bstree->root, 0, sizeof(AVLTreeNode) * 1);
    /* 初始化根节点 */
    {
        bstree->root->data = 0;
        bstree->root->left = NULL;
        bstree->root->right = NULL;
        bstree->root->parent = NULL;
    }
#else
    bstree->root = createAVLTreeNewNode(0, NULL);
    if(bstree->root == NULL)
    {
        return MALLOC_ERROR;
    }
#endif

    *pBstree = bstree;
    return ret;
}

/* 判断二叉搜索树为2 */
static int balanceBinarySearchTreeNodeHasTwoChildren(AVLTreeNode *node)
{
    return (node->left != NULL) && (node->right != NULL);
}
/* 判断二叉搜索树度为1 */
static int balanceBinarySearchTreeNodeHasOneChild(AVLTreeNode *node)
{
    return ((node->left == NULL) && (node->right != NULL)) || ((node->right == NULL) && (node->left != NULL));
}

/* 判断二叉搜索树度为0 */
static int balanceBinarySearchTreeNodeIsLeaf(AVLTreeNode *node)
{
    return (node->right == NULL) && (node->left == NULL);
}

/* 获取当前结点的前驱结点 中序遍历到结点的前一个结点 */
static AVLTreeNode * bstreeNodePreDecessor(AVLTreeNode * node)
{
    if (node->left != NULL)
    {
        /*前驱结点是在左子树的右子树的右子树...*/
        AVLTreeNode * travelNode = node->left;
        while (travelNode->right != NULL)
        {
            travelNode = travelNode->right;
        }
        return travelNode;       
    }
    /* 程序执行到这个地方 说明一定没有左子树 那就需要想父节点找 */
    while (node->parent != NULL && (node == node->parent->left))
    {
        node = node->parent;
    }
    /* node->parent == NULL */
    /* node == node->parent->right */
    return node->parent;
}

/* 获取当前结点的后继结点 */
static AVLTreeNode * bstreeNodeSuccessor(AVLTreeNode * node)
{
    if (node->right != NULL)
    {
        /* 后继结点是在右子树的左子树的左子树...*/
        AVLTreeNode * travelNode = node->right;
        while (travelNode->left != NULL)
        {
            travelNode = travelNode->left;
        }
        return travelNode;       
    }
    /* 程序执行到这个地方 说明一定没有右子树 那就需要想父节点找 */
    while (node->parent != NULL && (node == node->parent->right))
    {
        node = node->parent;
    }
    /* node->parent == NULL */
    /* node == node->parent->right */
    return node->parent;
}

/* 当前结点是父结点的左子树 */
static int AVLTreeCurrentNodeIsLeft(AVLTreeNode * node)
{
    return (node->parent != NULL) && (node == node->parent->left);
}
/* 当前结点是父结点的右子树 */
static int AVLTreeCurrentNodeIsRight(AVLTreeNode * node)
{
    return (node->parent != NULL) && (node == node->parent->right);
}

#if 0
static int compareFunc(ELEMENTTYPE val1, ELEMENTTYPE val2)
{
    return val1 - val2;
}
#endif

/* 创建结点 */
static AVLTreeNode * createAVLTreeNewNode(ELEMENTTYPE val, AVLTreeNode *parent)
{
    /* 分配根节点 */
    AVLTreeNode * newAVLNode = (AVLTreeNode *)malloc(sizeof(AVLTreeNode) * 1);
    if (newAVLNode == NULL)
    {
        return NULL;
    }
    /* 清楚脏数据 */
    memset (newAVLNode, 0, sizeof(AVLTreeNode) * 1);
    /* 初始化根节点 */
    {
        newAVLNode->data = 0;
        newAVLNode->height = 1;
        newAVLNode->left = NULL;
        newAVLNode->right = NULL;
        newAVLNode->parent = NULL;
    }
    /* 赋值 */
    newAVLNode->data = val;
    newAVLNode->parent = parent;
    return newAVLNode;
}

/* 获取AVL结点较高的子结点 */
static AVLTreeNode * AVLTreeNodeGetChildTaller(AVLTreeNode * node)
{
    /* 左子树高度 */
    int leftHeight = node->left == NULL ? 0 : node->left->height;
    /* 右子树高度 */
    int rightHeight = node->right == NULL ? 0 : node->right->height;
    if (leftHeight > rightHeight)
    {
        return node->left;
    }
    else if (leftHeight < rightHeight)
    {
        return node->right;
    }
    else
   {
        /* leftHeight = rightHeight */
        if (AVLTreeCurrentNodeIsLeft(node))
        {
            return node->left;
        }
        else if (AVLTreeCurrentNodeIsRight(node))
        {
            return node->right;
        }
   }
} 

/* 旋转公共代码部分 */
static int AVLTreeNodeRotate(BalanceBinarySearchTree  *pBstree, AVLTreeNode * grand, AVLTreeNode *parent, AVLTreeNode * child)
{
    /* p成为新的根结点 */
    parent->parent = grand->parent;
    
    if (AVLTreeCurrentNodeIsLeft(grand))
    {
        grand->parent->left = parent;
    }
    else if (AVLTreeCurrentNodeIsRight(grand))
    {
        grand->parent->right = parent;
    }
    else
    {
        /* p成为新的根结点 */
        pBstree->root = parent;
    }
    grand->parent = parent;
    if (child != NULL)
    {
        child->parent = grand;
    }
    /* 更新高度 */
    /* 先更新低的结点 */
    AVLTreeNodeUpdateHeight(grand);
    AVLTreeNodeUpdateHeight(child);
}

/* 左旋 */
static int AVLTreeCurrentNodeRatoteLeft(BalanceBinarySearchTree *pBstree, AVLTreeNode * grand)
{
    int ret = 0;
    AVLTreeNode * parent = grand->right;
    AVLTreeNode * child = parent->left;

    grand->right = child;
    parent->left = grand;

#if 0
    /* p成为新的根结点 */
    parent->parent = grand->parent;
    
    if (AVLTreeCurrentNodeIsLeft(grand))
    {
        grand->parent->left = parent;
    }
    else if (AVLTreeCurrentNodeIsRight(grand))
    {
        grand->parent->right = parent;
    }
    else
    {
        /* p成为新的根结点 */
        pBstree->root = parent;
    }
    grand->parent = parent;
    if (child != NULL)
    {
        child->parent = grand;
    }
    /* 更新高度 */
    /* 先更新低的结点 */
    AVLTreeNodeUpdateHeight(grand);
    AVLTreeNodeUpdateHeight(child);
#else
    AVLTreeNodeRotate(pBstree, grand, parent, child);
#endif
    return ret;
}

/* 右旋 */
static int AVLTreeCurrentNodeRatoteRight(BalanceBinarySearchTree *pBstree, AVLTreeNode * grand)
{
    int ret = 0;
    AVLTreeNode * parent = grand->left;
    AVLTreeNode * child = parent->right;

    grand->left = child;
    parent->right = grand;
#if 0
    /* p成为新的根结点 */
    parent->parent = grand->parent;
    
    if (AVLTreeCurrentNodeIsLeft(grand))
    {
        grand->parent->left = parent;
    }
    else if (AVLTreeCurrentNodeIsRight(grand))
    {
        grand->parent->right = parent;
    }
    else
    {
        /* p成为新的根结点 */
        pBstree->root = parent;
    }
    grand->parent = parent;
    if (child != NULL)
    {
        child->parent = grand;
    }
    /* 更新高度 */
    /* 先更新低的结点 */
    AVLTreeNodeUpdateHeight(grand);
    AVLTreeNodeUpdateHeight(child);
#else
    AVLTreeNodeRotate(pBstree, grand, parent, child);
#endif
    return ret;
}

/* AVL树结点调整平衡 */
/* node一定是最低的不平衡结点 */
static int AVLTreeNodeAdjustBalance(BalanceBinarySearchTree *pBstree, AVLTreeNode * node)
{
    /* LL LR RL RR */
    AVLTreeNode * parent = AVLTreeNodeGetChildTaller(node);
    AVLTreeNode * child = AVLTreeNodeGetChildTaller(parent);
    /* L */
    if (AVLTreeCurrentNodeIsLeft(parent))
    {
        if (AVLTreeCurrentNodeIsLeft(child))
        {
            /* LL 右旋 */
            AVLTreeCurrentNodeRatoteRight(pBstree, node);
        }
        else if (AVLTreeCurrentNodeIsRight(child))
        {
            /* LR */
            AVLTreeCurrentNodeRatoteLeft(pBstree, parent);
            AVLTreeCurrentNodeRatoteRight(pBstree, node  );
        }
    }
    else
    {
        if (AVLTreeCurrentNodeIsLeft(child))
        {
            /* RL */
            AVLTreeCurrentNodeRatoteRight(pBstree, parent);
            AVLTreeCurrentNodeRatoteLeft(pBstree, node);
        }
        else if (AVLTreeCurrentNodeIsRight(child))
        {
            /* RR */ 
            AVLTreeCurrentNodeRatoteLeft(pBstree, node);
        }

    }
}

/* 删除结点之后要做的事情 */
static int removeNodeAfter(BalanceBinarySearchTree *pBstree, AVLTreeNode * node)
{
    int ret = 0;
    /* 时间复杂度是O(logN) */
    while ( (node = node->parent) != NULL)
    {
        /* 程序执行到这里面的时候, 一定不止一个结点. */
        if (AVLTreeNodeIsBalanced(node))
        {
            /* 如果结点是平衡的. 那就更新高度. */
            AVLTreeNodeUpdateHeight(node);
        }
        else
        {
            /* node是最低不平衡结点 */
            /* 调整平衡 */
            AVLTreeNodeAdjustBalance(pBstree, node);
        }
    }
    return ret;
}

/* 添加结点后的操作 */
/* 新添加的结点一定是叶子结点 */
static int insertNodeAfter(BalanceBinarySearchTree *pBstree, AVLTreeNode * node)
{
    int ret = 0;
    while ((node = node->parent) != NULL)
    {
        /* 程序执行到这里一定不止这一个结点 */
        if (AVLTreeNodeIsBalanced(node))
        {
            /* 如果结点是平衡的 那就更新高度 */
            AVLTreeNodeUpdateHeight(node);
        }
        else
        {
            /* node是最低不平衡结点 */
            /* 调整平衡 */
            AVLTreeNodeAdjustBalance(pBstree, node);

            /* 调整完最低的不平衡结点，上面的不平衡结点已经平衡 */
            break;
        }
    }
    return ret;
}

/* 比较两个整数的最大值 */
static int tmpMax(int val1, int val2)
{
    return val1 - val2 >= 0 ? val1 : val2;
}

/* 更新结点的高度 */
static int AVLTreeNodeUpdateHeight(AVLTreeNode *node)
{
    int ret = 0;
#if 1   
    if (node == NULL)
    {
        return ret;
    }
    /* 左子树的高度 */
    int leftHeight = node->left == NULL ? 0 : node->left->height;
    /* 右子树的高度 */
    int rightHeight = node->right == NULL ? 0 : node->right->height;
    node->height = 1 + tmpMax(leftHeight, rightHeight);
   
#else

    AVLTreeNodeBalanceFactor(node) >= 0 ? 1 + node.
#endif
    return ret;
}

/* 计算结点的平衡因子 */
/* 左子树 - 右子树 */
static int AVLTreeNodeBalanceFactor(AVLTreeNode *node)
{
    /* 左子树的高度 */
    int leftHeight = node->left == NULL ? 0 : node->left->height;

    /* 右子树的高度 */
    int rightHeight = node->right == NULL ? 0 : node->right->height;

    return leftHeight - rightHeight;
}

/* 判断结果是否平衡*/
static int AVLTreeNodeIsBalanced(AVLTreeNode *node)
{
    int  nodeFactor = abs(AVLTreeNodeBalanceFactor(node));
    if(nodeFactor <= 1)
    {
        return true;
    }
    else
    {
        return false;
    }
}


/* 二叉搜索树的插入 */
int balanceBinarySearchTreeInsert(BalanceBinarySearchTree *pBstree, ELEMENTTYPE val)
{
    int ret = 1;
    /* 空树 */
    if (pBstree->size == 0)
    {
        /* 更新树的结点 */
        (pBstree->size)++;
        pBstree->root->data = val;
        insertNodeAfter(pBstree, pBstree->root);
        return ret;
    }

    /* travelNode指向根节点 */
    AVLTreeNode * travelNode = pBstree->root;
    AVLTreeNode * parentNode = pBstree->root;
    /* 比较传入的值 确定符号 到底放在左边右边 */
    int cmp = 0;
    while (travelNode != NULL)
    {
        /* 标记父节点 */
        parentNode = travelNode;
        cmp = pBstree->compareFunc(val, travelNode->data);
        /* 插入元素 < 遍历的结点 */
        if (cmp < 0)
        {
            travelNode = travelNode->left;
        }
        /* 插入元素 > 遍历的结点 */
        else if (cmp > 0)
        {
            travelNode = travelNode->right;
        }
        else
        {
            /* 插入元素 = 遍历的结点 */
            printf("添加失败\n已有此联系人\n");
            return 0;
        }
    }
#if 0
    AVLTreeNode * newAVLNode = (AVLTreeNode *)malloc(sizeof(AVLTreeNode) * 1);
    if (newAVLNode == NULL)
    {
        return MALLOC_ERROR;
    }
    /* 清楚脏数据 */
    memset (newAVLNode, 0, sizeof(AVLTreeNode) * 1);
    /* 初始化根节点 */
    {
        newAVLNode->data = 0;
        newAVLNode->left = NULL;
        newAVLNode->right = NULL;
        newAVLNode->parent = NULL;
    }
    /* 新结点赋值 */
    newAVLNode->data = val;
#else
    AVLTreeNode * newAVLNode = createAVLTreeNewNode(val, parentNode);
#endif

/* 挂在左子树 */
    if (cmp < 0)
    {
        parentNode->left = newAVLNode;
    }
    else
    {
        parentNode->right = newAVLNode;
    }
    /* 添加之后的调整 */
    insertNodeAfter(pBstree, newAVLNode);           
#if 0
    /* 新结点的parent赋值 */
    newAVLNode->parent = parentNode;
#endif
    /* 更新树的结点 */
    (pBstree->size)++;
    return ret;
}

/* 根据指定的值获取二叉搜索树的结点 */
static AVLTreeNode * baseAppointValGetAVLTreeNode(BalanceBinarySearchTree *pBstree, ELEMENTTYPE val)
{
    AVLTreeNode * travelNode = pBstree->root;
    int cmp = 0;
    while (travelNode != NULL)
    {
        cmp = pBstree->compareFunc(val, travelNode->data);
        if (cmp < 0)
        {
            travelNode = travelNode->left;
        }
        else if (cmp > 0)
        {
            travelNode = travelNode->right;
        }
        else
        {
            /* 找到 */
            return travelNode;
        }
    }
    return NULL;
}
/* 二叉搜索时是否包含指定元素 */
int balanceBinarySearchTreeIsContainAppointVal(BalanceBinarySearchTree *pBstree, ELEMENTTYPE val)
{
    return baseAppointValGetAVLTreeNode(pBstree, val) == NULL ? 0 : 1;
}


/* 二叉搜索树的前序遍历 */
/* 根节点 左子树 右子树 */
static int preOrderTravel(BalanceBinarySearchTree *pBstree, AVLTreeNode *node)
{
    int ret = 0;
    if(node == NULL)
    {
        return ret;
    }
    /* 根节点 */
    pBstree->printFunc(node->data);
    /* 左子树 */
    inOrderTravel(pBstree, node->left);   
    /* 右子树 */
    inOrderTravel(pBstree, node->right);

}
int balanceBinarySearchTreePreOrderTravel(BalanceBinarySearchTree *pBstree)
{
    int ret = 0;
    preOrderTravel(pBstree, pBstree->root);
    return ret;

}

/* 二叉搜索树的中序遍历 */
/* 左子树 根节点 右子树 */
static int inOrderTravel(BalanceBinarySearchTree *pBstree, AVLTreeNode *node)
{
    int ret = 0;
    if(node == NULL)
    {
        return ret;
    }
    /* 左子树 */
    inOrderTravel(pBstree, node->left);
    /* 根节点 */
    pBstree->printFunc(node->data);
    /* 右子树 */
    inOrderTravel(pBstree, node->right);

}
int balanceBinarySearchTreeInOrderTravel(BalanceBinarySearchTree *pBstree)
{
    int ret = 0;
    inOrderTravel(pBstree, pBstree->root);
    return ret;
}

/* 二叉搜索树的后序遍历 */
/* 左子树 右子树 根节点 */
static int postOrderTravel(BalanceBinarySearchTree *pBstree, AVLTreeNode *node)
{
    int ret = 0;
    if(node == NULL)
    {
        return ret;
    }
    /* 左子树 */
    inOrderTravel(pBstree, node->left);
    /* 右子树 */
    inOrderTravel(pBstree, node->right);   
    /* 根节点 */
    pBstree->printFunc(node->data);
}
int balanceBinarySearchTreePosOrderTravel(BalanceBinarySearchTree *pBstree)
{
    int ret = 0;
    postOrderTravel(pBstree, pBstree->root);
    return ret;
}

/* 二叉搜索树的层序遍历 */
int balanceBinarySearchTreeLevelOrderTravel(BalanceBinarySearchTree *pBstree)
{
    int ret = 0;
    doubleLinkListQueue * pQueue = NULL;
    doubleLinkListQueueInit(&pQueue);
    
    /* 根节点入队 */
    doubleLinkListQueuePush(pQueue, pBstree->root);

    /* 判断队列是否为空 */
    AVLTreeNode * travelNode = NULL;
    while(!doubleLinkListQueueIsEmpty(pQueue))
    {
        doubleLinkListQueueTop(pQueue, (void **)&travelNode);
    #if 0
        printf("data:%d\n", travelNode->data);
    #else
        pBstree->printFunc(travelNode->data);
    #endif
        doubleLinkListQueuePop(pQueue);

        /* 将左子树入队 */
        if (travelNode->left !=NULL)
        {
            doubleLinkListQueuePush(pQueue, travelNode->left);
        }

        /* 将右子树入队 */
        if (travelNode->right != NULL)
        {
            doubleLinkListQueuePush(pQueue, travelNode->right);
        }
    }
    /* 释放队列 */
    doubleLinkListQueueDestorty(pQueue);
    return ret;
}

/* 获取二叉搜索树的结点个数 */
int balanceBinarySearchTreeGetNodeSize(BalanceBinarySearchTree *pBstree, int *pSize)
{
    if(pBstree == NULL)
    {
        return 0;
    }
    if(pSize)
    {
        *pSize = pBstree->size;
    }
    return pBstree->size;
}

/* 获取二叉搜树的高度 */
int balanceBinarySearchTreeGetHeight(BalanceBinarySearchTree *pBstree, int *pHeight)
{
    int ret = 0;
    if (pBstree == NULL)
    {
        return NULL_PTR;
    }
#if 0
    /* 判断是否为空树 */
    if (pBstree->size == 0)
    {
        return 0;
    }
    *pHeight = pBstree->root->height;
    return pBstree->root->height;
#else
    /* 空树 */
    int height = 0;
    if (pBstree->size == 0)
    {
        return 0;
    }

    doubleLinkListQueue * pQueue = NULL;
    doubleLinkListQueueInit(&pQueue);
    
    /* 根节点入队 */
    doubleLinkListQueuePush(pQueue, pBstree->root);

    /* 队列的大小 */
    int levelSize = 1;
    AVLTreeNode * nodeVal = NULL;
    //int queueSize = 0;
    /* 判断队列是否为空 */   
    //while (doubleLinkListQueueGetSize(pQueue, &queueSize))
    while (!doubleLinkListQueueIsEmpty(pQueue))
    {
        doubleLinkListQueueTop(pQueue, (void **)&nodeVal);
        doubleLinkListQueuePop(pQueue);
        levelSize--;

        /* 左子树不为空 */
        if (nodeVal->left != NULL)
        {
            doubleLinkListQueuePush(pQueue, nodeVal->left);
        }

        /* 右子树不为空 */
        if (nodeVal->right != NULL)
        {
            doubleLinkListQueuePush(pQueue, nodeVal->right);
        }

        /* 树的当前层结点遍历结束 */
        if (levelSize == 0)
        {
            height++;
            doubleLinkListQueueGetSize(pQueue, &levelSize);
        }
    }
    /* 解引用 */
    *pHeight = height;

    /* 释放队列 */
    doubleLinkListQueueDestorty(pQueue);
#endif
    return ret;
}

/* 二叉搜索树删除指定的节点 */
static int balanceBinarySearchTreeDelNode(BalanceBinarySearchTree *pBstree, AVLTreeNode *node)
{
    int ret = 0;
    if(node == NULL)
    {
        return ret;
    }

    /* 树的结点减一 */
    (pBstree->size) --;
    if (balanceBinarySearchTreeNodeHasTwoChildren(node))
    {
        /* 找到前驱结点 */
        AVLTreeNode * preNode = bstreeNodePreDecessor(node);
        node->data = preNode->data;
        node = preNode;
    }

    /* 程序执行到这里 要删除的结点度为1或0 */
#if 0
    if (balanceBinarySearchTreeNodeHasOneChild(node))
    {
        
    }

    if (balanceBinarySearchTreeNodeIsLeaf(node))
    {
        
    }
#else
    /* 假设node结点度为1 它的child要么是左要么是右 */
    /* */
    AVLTreeNode * child = node->left != NULL ? node->left : node->right;
    AVLTreeNode * delNode = NULL;
    if (child)
    {
        /* 度为1 */
        child->parent = node->parent;
        if (node->parent == NULL)
        {
            /* 度为1 且 它是根结点 */
            pBstree->root = child;
            delNode = node; 
            /* 删除的结点 */          
            removeNodeAfter(pBstree, delNode);
        }
        else
        {
            /* 度为1 且 它不是根结点 */
            if (node == node->parent->left)
            {
                node->parent->left = child;
            }
            else if (node == node->parent->right)
            {
                node->parent->right = child;
            }
            delNode = node;
            /* 删除的结点 */
            removeNodeAfter(pBstree, delNode);
        }
    }
    else
    {
        /* 度为0 */
        if (node->parent == NULL)
        {
            /* 度为0 且是根结点 */
            delNode = node;
            /* 删除的结点 */
            removeNodeAfter(pBstree, delNode);
        }
        else
        {
            if (node == node->parent->left)
            {
                node->parent->left = NULL;
            }
            else if (node == node->parent->right)
            {
                node->parent->right = NULL;
            }
            delNode = node;
            /* 删除的结点 */
            removeNodeAfter(pBstree, delNode);
        }       
    }

    if (delNode)
    {
        free(delNode);
        delNode = NULL;
    }
    
    return ret;
   

#endif
}

/* 二叉搜索树的删除 */
int balanceBinarySearchTreeDelete(BalanceBinarySearchTree *pBstree, ELEMENTTYPE val)
{
    if (pBstree == NULL)
    {
        return NULL_PTR;
    }
    return balanceBinarySearchTreeDelNode(pBstree, baseAppointValGetAVLTreeNode(pBstree, val));
}

/* 判断树是否是完全二叉树 */
int balanceBinarySearchTreeIsComplete(BalanceBinarySearchTree *pBstree)
{
    return 0;
}

/* 二叉搜索树的销毁*/
int balanceBinarySearchTreeDestory(BalanceBinarySearchTree * pBstree)
{
    if(pBstree == NULL)
    {
        return NULL_PTR;
    }

    int ret;
    doubleLinkListQueue * pQueue = NULL;
    doubleLinkListQueueInit(&pQueue);

    /* 根节点入队 */
    doubleLinkListQueuePush(pQueue, pBstree->root);

    AVLTreeNode * nodeVal = NULL;
    while (doubleLinkListQueueIsEmpty(pQueue))
    {
        doubleLinkListQueueTop(pQueue, (void **)&nodeVal);
        doubleLinkListQueuePop(pQueue);
        /* 左子树不为空 */
        if (nodeVal->left != NULL)
        {
            doubleLinkListQueuePush(pQueue, nodeVal->left);
        }

        /* 右子树不为空 */
        if (nodeVal->right != NULL)
        {
            doubleLinkListQueuePush(pQueue, nodeVal->right);
        }
        /* 最后释放 */
        if(nodeVal)
        {
            free(nodeVal);
            nodeVal = NULL;
        }
    }

    /* 释放队列 */
    doubleLinkListQueueDestorty(pQueue);

    /* 释放树 */
    if(pBstree)
    {
        free(pBstree);
        pBstree = NULL;
    }
    return ret;

}