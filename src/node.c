#ifndef FLASHLIGHT_NODE
#define FLASHLIGHT_NODE
#include "node.h"

int f_node_new(f_node** out, size_t offset)
{
  f_node* init = malloc(sizeof(*init));
  if (init == NULL)
  {
    return -1;
  }

  init->offset = offset;
  init->next = NULL;

  *out = init;

  return 0;
}

// todo: don't think this is relevant. should probably 
// remove it soon.
void f_node_append(f_node** parent, f_node* child)
{
  f_node* head = *parent;
  if (head == NULL)
  {
    *parent = child;
  }
  else
  {
    while (head->next != NULL)
    {
      head = head->next;
    }

    head->next = child;
  }
}

void f_node_free(f_node** node)
{
  f_node* tmp;
  f_node* head = *node;

  while (head != NULL)
  {
    tmp = head;
    head = head->next;
    free(tmp);
  }
  *node = NULL;
}

#endif