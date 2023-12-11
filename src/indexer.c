#ifndef FLASHLIGHT_INDEXER
#define FLASHLIGHT_INDEXER
#include "indexer.h"

int f_indexer_chunks_init(f_indexer_chunks** out, int concurrency, size_t buffer_size, size_t total_bytes, size_t from, size_t to)
{
  f_indexer_chunks* init = malloc(sizeof(*init));
  if (init == NULL)
  {
    return -1;
  }

  size_t bytes_count = to - from;

  unsigned int chunks_count = (unsigned int) ceil(bytes_count / (double) (buffer_size));//);

  if (chunks_count == 0)
  {
    concurrency = 1;
  }
  else if (concurrency > chunks_count)
  {
    concurrency = (int) chunks_count;
  }

  init->concurrency = concurrency;
  init->len = chunks_count;
  init->chunks = malloc(sizeof(f_indexer_chunk*) * chunks_count);

  if (init->chunks == NULL)
  {
    free(init);
    return -1;
  }

  for(int cc=0; cc<chunks_count; cc++)
  {
    unsigned int start_position = from + (cc * buffer_size);

    if (start_position + buffer_size > bytes_count + from)
    {
      buffer_size = (bytes_count + from) - start_position;
    }

    f_indexer_chunk* chunk = malloc(sizeof(f_indexer_chunk));
    if (chunk == NULL)
    {
      free(init->chunks);
      free(init);
      return -1;
    }

    chunk->index = cc;
    chunk->from = start_position;
    chunk->count = buffer_size;

    init->chunks[cc] = chunk;
  }

  *out = init;
  return 0;
}

void f_indexer_chunks_free(f_indexer_chunks* index)
{
  for (int i=0; i<index->len; i++)
  {
    free(index->chunks[i]);
  }

  free(index->chunks);
  free(index);
  index = NULL;
}

int f_indexer_threads_init(f_indexer_threads** out, int threads, size_t total_bytes_count, size_t buffer_size, size_t offset)
{
  f_indexer_threads* init = malloc(sizeof(*init));

  if (init == NULL)
  {
    return -1;
  }

  unsigned int chunks_count = (unsigned int) ceil(total_bytes_count / (double) buffer_size);

  if (chunks_count == 0)
  {
    // 1 thread/buffer can open this file.
    threads = 1;
  }
  else if (threads > chunks_count)
  {
    // small files.
    threads = (int) chunks_count;
  }

  // established number of threads, let's allocate.
  init->len = threads;
  init->threads = malloc(sizeof(f_indexer_thread) * threads);

  if (init->threads == NULL)
  {
    free(init);
    return -1;
  }

  unsigned long int pages = (unsigned long int) ceil(total_bytes_count / threads);

  for (int i=0; i<threads; i++)
  {
    unsigned long int start_position = (i * pages);
    start_position += i;

    if (start_position + pages > total_bytes_count)
    {
      pages = (total_bytes_count - start_position);
    }

    unsigned long int to = start_position + pages;

    f_indexer_thread index = {
      .from = start_position + offset,
      .to = to + offset,
      .total_bytes_count = total_bytes_count,
      .buffer_size = buffer_size,
    };

    init->threads[i] = index;
  }

  *out = init;
  return 0;
}

void f_indexer_threads_free(f_indexer_threads* index)
{
  free(index->threads);
  free(index);
}

#endif
