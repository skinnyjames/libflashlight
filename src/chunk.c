#ifndef FLASHLIGHT_CHUNK
#define FLASHLIGHT_CHUNK
#include "chunk.h"

int f_chunk_new(f_chunk** out, unsigned long int current, f_bytes_node* firstref, f_bytes_node* lastref)
{
  f_chunk* init = malloc(sizeof(*init));
  if (init == NULL)
  {
    return -1;
  }

  init->current = current;
  init->first = firstref;
  init->last = lastref;
  init->line_count = 0;
  if (firstref != NULL)
  {
    init->empty = false;
  }
  else
  {
    init->empty = true;
  }

  *out = init;
  return 0;
}

void f_chunk_free(f_chunk** chunk)
{
  f_chunk* head = *chunk;
  free(head);
  *chunk = NULL;
}

/* 
  free's a chunk and all of its children.
  you probably don't want to call this,
  since a chunk is an aggregation of pointers to pointers.
*/
void f_chunk_free_all(f_chunk* chunk)
{
  f_bytes_node_free(&chunk->first);
  f_chunk_free(&chunk);
}

void f_chunks_swap(f_bytes_node** last, f_bytes_node** child)
{
  (*last)->next = *child;
}

int f_chunk_array_reverse_reduce(f_chunk** out, int idx, f_chunk** chunks, size_t len)
{ 
  f_chunk* result = malloc(sizeof(f_chunk));

  if (result == NULL)
  {
    return -1;
  }

  if (len == 1)
  {
    *result = *chunks[0];
    (result)->current = idx;
    *out = result;
    return 0;
  }

  // clone first chunks last.
  (result)->last = NULL;
  (result)->first = NULL;

  f_bytes_node* current = NULL;

  int i = 0;
  bool head = true;
  bool empty = true;

  while (i < len)
  {
    if (chunks[i]->empty)
    {
      i++;
      continue;
    }

    if (current == NULL)
    {
      current = chunks[i]->first;

      if (head)
      {
        result->last = chunks[i]->last;
        head = false;
      }
    }
    else
    {
      f_bytes_node* last = chunks[i]->last;
      f_bytes_node* parent = chunks[i]->first;

      f_chunks_swap(&last, &current);
      result->first = parent;
      current = parent;
    }
    empty = false;
    i++;
  }

  result->empty = empty;
  result->current = idx;
  *out = result;
  return 0;
}

int f_chunk_array_new(f_chunk*** out, unsigned long len)
{
  f_chunk** arr = malloc(sizeof(*arr) * len);

  if (arr == NULL)
  {
    return -1;
  }

  *out = arr;
  return 0;
}

void f_chunk_array_free(f_chunk** chunks, size_t len)
{
  for (int i=0; i<len; i++)
  {
    free(chunks[i]);
  }
  free(chunks);
}

void f_chunk_array_free_all(f_chunk** chunks, size_t len)
{
  f_chunk_free_all(chunks[0]);
  free(chunks);
}

#endif
