#ifndef P_LIST_H
#define P_LIST_H

#define PLIST_ENTRY(type) \
type *next;type *prev

#define PListInit(list) \
(list)->next = (list); \
(list)->prev = (list);

#define PListFirst(list) ((list)->next == (list) ? PNULL : (list)->next)
#define PListLast(list) ((list)->prev == (list) ? PNULL : (list)->prev)

#define PListAdd(list, node) \
node->next = (list);\
(list)->prev->next = node;\
node->prev = (list)->prev;\
(list)->prev = node;

#define PListDel(node) \
(node)->prev->next = (node)->next;\
(node)->next->prev = (node)->prev;

#define _P_LIST_TMP_NODE(node, line) node##line

#define P_LIST_TMP_NODE(node, line) _P_LIST_TMP_NODE(node, line)

//P_LIST_TMP_NODE 用于记录next节点，用于保证遍历过程中Del节点不会导致错误
#if 1
#define PListForeach(list, node) \
void *P_LIST_TMP_NODE(node, __LINE__); \
for(node = (list)->next, P_LIST_TMP_NODE(node, __LINE__) = node->next; node != (list); node = P_LIST_TMP_NODE(node, __LINE__), P_LIST_TMP_NODE(node, __LINE__) = node->next)
#else
#define PListForeach(list, node) \
for(node = (list)->next; node != (list); node = node->next)
#endif

#endif // P_LIST_H
