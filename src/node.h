#ifndef FLASHLIGHT_NODE_H
#define FLASHLIGHT_NODE_H

/** @struct FNode
* @brief an offset list, useful for in memory indexes
* @var offset
* the offset of the this line
* @var next
* the next FNode / offset
*/
typedef struct FNode
{
  size_t offset;
  struct FNode* next;
} f_node;

/**
  Initializes a new node
  @param out the node to init
  @param offset the offset of the new line
  @return non zero for error
*/
int f_node_new(f_node** out, size_t offset);

/**
  Append a new node to an existing node
  @param parent the existing node
  @param child the new node
*/
void f_node_append(f_node** parent, f_node* child);

/**
  Free a node
  @param node node to free
*/
void f_node_free(f_node** node);

#endif
