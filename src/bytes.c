#ifndef FLASHLIGHT_BYTES
#define FLASHLIGHT_BYTES
#include "bytes.h"

int f_bytes_new(f_bytes** out, bool atomic, size_t offset)
{
  f_bytes* init = malloc(sizeof(f_bytes));
  if (init == NULL)
  {
    return -1;
  }

  init->atomic = atomic;
  init->offset = offset;
  *out = init;
  return 0;
}

void f_bytes_free(f_bytes** bytes)
{
  free(*bytes);
  *bytes = NULL;
}

int f_bytes_node_new(f_bytes_node** out, f_bytes* bytes)
{
  f_bytes_node* init = malloc(sizeof(*init));
  if (init == NULL)
  {
    return -1;
  }

  init->bytes = bytes;
  init->next = NULL;

  *out = init;
  return 0;
}

int f_bytes_node_clone(f_bytes_node**out, f_bytes_node* node)
{
  return f_bytes_node_new(out, node->bytes);
}

void f_bytes_node_free(f_bytes_node** node)
{
  f_bytes_node* tmp;
  f_bytes_node* head = *node;
  while (head != NULL)
  {
    tmp = head;
    head = head->next;
    f_bytes_free(&tmp->bytes);
    free(tmp);
  }
  *node = NULL;
}

/*
  { 8, false } -> { 6, true } -> { 2, false } -> { 1, true}

  {3, 14}
*/
int f_node_from_reversed_bytes_node(f_node** out, f_bytes_node** bytes_head)
{
  f_bytes_node* prev = NULL;
  f_bytes_node* current = *bytes_head;

  int last = 0;
  size_t offset = 0ul;

  while (current != NULL)
  {
    if (current->bytes->atomic == false) 
    {
      prev = current;
      current = current->next;
      continue;
    } 
    else if (prev != NULL) 
    {
      offset = prev->bytes->offset + current->bytes->offset;
      prev = NULL;
    }
    else
    {
      offset = current->bytes->offset;
    }

    if (last == 0)
    {
      if (f_node_new(out, offset) == -1)
      {
        return -1;
      };
      last = 1;
    } 
    else
    {
      // create and prepend node
      f_node* new_node;
      if (f_node_new(&new_node, offset) == -1)
      {
        return -1;
      }

      f_node* tmp = *out;
      new_node->next = tmp;
      *out = new_node;
    }

    current = current->next;
  }
  // prepend 0 to the node.
  f_node* first_node;
  if (f_node_new(&first_node, 0ul) == -1)
  {
    return -1;
  }
  
  f_node* atmp = *out;
  first_node->next = atmp;
  *out = first_node;  
  return 0;
}

int f_bytes_node_prepend(f_bytes_node** head, f_bytes_node** parent)
{
  if (*head == NULL)
  {
    *head = *parent;
  }
  else
  {
    f_bytes_node* tmp = *head;

    (*parent)->next = tmp;
    *head = *parent;
  }
  return 0;
}

#endif
