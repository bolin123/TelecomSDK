#ifndef VT_LIST_H
#define VT_LIST_H

#define VTLIST_ENTRY(type) \
type *next;type *prev

#define VTListInit(list) \
(list)->next = (list); \
(list)->prev = (list);

#define VTListFirst(list) ((list)->next == (list) ? NULL : (list)->next)
#define VTListLast(list) ((list)->prev == (list) ? NULL : (list)->prev)

#define VTListAdd(list, node) \
node->next = (list);\
(list)->prev->next = node;\
node->prev = (list)->prev;\
(list)->prev = node;

#define VTListDel(node) \
(node)->prev->next = (node)->next;\
(node)->next->prev = (node)->prev;

#define VT_LIST_TMP_NODE(node, line) \
node##line

//VT_LIST_TMP_NODE 用于记录next节点，用于保证遍历过程中Del节点不会导致错误
#if 1
#define VTListForeach(list, node) \
void *VT_LIST_TMP_NODE(node, line); \
for(node = (list)->next, VT_LIST_TMP_NODE(node, line) = node->next; node != (list); node = VT_LIST_TMP_NODE(node, line), VT_LIST_TMP_NODE(node, line) = node->next)
#else
#define VTListForeach(list, node) \
for(node = (list)->next; node != (list); node = node->next)
#endif

#endif // VT_LIST_H
