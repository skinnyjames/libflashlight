#ifndef FLASHLIGHT_INDEXERS_TEXT
#define FLASHLIGHT_INDEXERS_TEXT
#include <pthread.h>
#include <libdill.h>
#include "text_indexer.h"

/*
  well, for this one i couldn't resist the easy route.
  https://codereview.stackexchange.com/questions/29198/random-string-generator-in-c
*/
static void rand_string(char* str, size_t size)
{
  const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJK...";
  if (size) 
  {
    --size;
    for (size_t n = 0; n < size; n++)
    {
      int key = rand() % (int) (sizeof charset - 1);
      str[n] = charset[key];
    }
    str[size] = '\0';
  }
}

coroutine void f_index_text_bytes(int fd, int done, f_indexer_chunk* ic, int thread)
{
  size_t buffer_size = ic->count;
  size_t total_bytes_offset = ic->from;

  // printf("count: %zu - [%u] from: %zu to: %zu\n", ic->count, ic->index, ic->from, ic->from + ic->count);

  uint8_t* buffer = malloc(sizeof(*buffer) * buffer_size);

  f_bytes_node* last_chunk_node = NULL;
  f_bytes_node* start_node = NULL;

  ssize_t bytes_read;
  size_t bytes_offset = 0ul;
  bool head = true;
  unsigned int line_count = 0u;

  bytes_read = pread(fd, buffer, buffer_size, total_bytes_offset);

  if (bytes_read == -1)
  {
    perror("failed to pread");
    exit(1);
  }

  int pos = 0;

  while (pos < bytes_read)
  {
    total_bytes_offset++;
    if ((char) buffer[pos] == '\n')
    {
      line_count++;

      f_bytes* bytes;
      f_bytes_new(&bytes, true, total_bytes_offset);

      f_bytes_node* node;
      f_bytes_node_new(&node, bytes);

      if (head)
      {
        last_chunk_node = node;
        head = false;
      }
  
      if (start_node == NULL) 
      {
        start_node = node;
      }
      else 
      {
        f_bytes_node_prepend(&start_node, &node);
      }
    }

    pos++;
  }

  free(buffer);
  f_chunk* chunk;
  f_chunk_new(&chunk, ic->index, start_node, last_chunk_node);
  chunk->line_count = line_count;

  if (chsend(done, &chunk, sizeof(chunk), -1) != 0)
  {
    perror("couldn't send channel message");
  }
}

void* f_index_text_chunk(void* payload)
{
  f_text_thread* tthread = (f_text_thread*) payload;

  unsigned int line_count = 0u;

  f_indexer_chunks* ic;
  f_indexer_chunks_init(&ic, tthread->concurrency, tthread->buffer_size, tthread->total_bytes_count, tthread->from, tthread->to);
  // printf("[%d] total bytes: %zu, from: %zu, to: %zu, len: %u\n", tthread->thread, tthread->total_bytes_count, tthread->from, tthread->to, ic->len);

  f_chunk** chunk_array;
  f_chunk_array_new(&chunk_array, ic->len);

  int chunks_finished = 0;
  int b = bundle();
  int chv[2];
  int rc = chmake(chv);
  if (rc == -1)
  {
    perror("cannot create channel");
  }

  int send = chv[0];
  int recv = chv[1];

  for (int i=0; i<ic->len; i+=ic->concurrency)
  {
    for (int c=0; c<ic->concurrency; c++)
    {
      unsigned long int index = i + c;

      if (index >= ic->len)
      {
        break;
      }

      f_indexer_chunk* chunk = ic->chunks[index];

      // printf("[%d] [%lu] [cc: %d] [buf: %zu] chunk start %u [total: %zu]\n", tthread->thread, index, c, chunk->count, chunk->from, chunk->from + chunk->count);

      if (bundle_go(b, f_index_text_bytes(tthread->fd, send, chunk, tthread->thread)) == -1)
      {
        perror("cannot run coroutine");
        exit(1);
      }
    }

    for (int c=0; c<ic->concurrency; c++)
    {
      unsigned long int index = i + c;

      if (index >= ic->len)
      {
        break;
      }

      f_chunk* chunk;
      if (chrecv(recv, &chunk, sizeof(chunk), -1) != 0)
      {
        perror("cannot receive chunk");
      }

      line_count += chunk->line_count;
      chunk_array[chunk->current] = chunk;
      chunks_finished++;
    }

    double prog = ((double) (chunks_finished) / ic->len);
    tthread->progress = prog;
  }

  if (hclose(send) == -1)
  {
    perror("couldn't close outbound channel");
  }

  if (hclose(recv) == -1)
  {
    perror("couldn't close inbound channel");
  }

  if (hclose(b) == -1)
  {
    perror("couldn't close channel bundle");
  }

  f_chunk* ret;
  f_chunk_array_reverse_reduce(&ret, tthread->thread, chunk_array, ic->len);
  ret->line_count = line_count;

  f_chunk_array_free(chunk_array, ic->len);
  f_indexer_chunks_free(ic);
  return (void*) ret;
}

/*
  entry point for this indexer.
*/
f_index* f_index_text_file(f_indexer indexer)
{
  FILE* fp;
  int fd;
  int flags;
  unsigned long int total_bytes_count;
  f_chunk* result_chunk;

  // get filehandle and total bytes. make nonblocking.
  fp = fopen(indexer.filename, "rb");
  fd = fileno(fp);
  flags = fcntl(fd, F_GETFL, 0);
  fcntl(fd, F_SETFL, flags | O_NONBLOCK);
  fseek(fp, 0, SEEK_END);
  total_bytes_count = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  double reported_progress = 0.0;
  size_t max_bytes_per_iteration = indexer.max_bytes_per_iteration;
  unsigned int thread_it_count = (unsigned int) ceil(total_bytes_count / (double) (max_bytes_per_iteration));
  if (thread_it_count == 0)
  {
    thread_it_count = 1;
  }

  max_bytes_per_iteration--;

  f_lookup_file* lookup = NULL;
  size_t index_filename_len = 12 + strlen(indexer.lookup_dir);
  char random[10];
  char* index_filename = malloc(sizeof(char) * index_filename_len);
  rand_string(random, 10);
  cwk_path_join(indexer.lookup_dir, random, index_filename, index_filename_len);

  for (int itc=thread_it_count - 1; itc>=0; itc--)
  {
    unsigned long int thread_it_start = itc * max_bytes_per_iteration;

    if (thread_it_start != 0)
    {
      thread_it_start++;
    }

    if (thread_it_start + max_bytes_per_iteration > total_bytes_count)
    {
      max_bytes_per_iteration = total_bytes_count - thread_it_start;
    }

    /* 
      lets try to iterate over x bytes at one time to keep consistent
      memory usage...
    */

    f_indexer_threads* it;
    f_indexer_threads_init(&it, indexer.threads, max_bytes_per_iteration, indexer.buffer_size, thread_it_start);

    // setup pthreads and thread chunks array.
    f_chunk** chunks;
    f_chunk_array_new(&chunks, it->len);

    f_text_thread** tthreads = malloc(sizeof(*tthreads) * it->len);
    pthread_t* thread_ids = malloc(sizeof(pthread_t) * it->len);

    unsigned int line_count = 0u;

    // spawn threads.
    for (int i=0; i<it->len; i++)
    {
      // add extra container.
      f_text_thread* tthread = malloc(sizeof(*tthread));
      tthread->fd = fd;
      tthread->from = it->threads[i].from;
      tthread->to = it->threads[i].to;
      tthread->total_bytes_count = total_bytes_count;
      tthread->buffer_size = it->threads[i].buffer_size;
      tthread->concurrency = indexer.concurrency;
      tthread->thread = i;
      tthread->progress = (double) 0.0;

      // printf("[%d] total bytes: %zu, from: %zu, to: %zu\n\n", i, max_bytes_per_iteration, tthread->from, tthread->to);

      tthreads[i] = tthread;
      pthread_create(&thread_ids[i], NULL, f_index_text_chunk, tthreads[i]);
    }

    double report = 0.0;
    // join threads.
    for (int i=0; i<it->len; i++)
    {
      bool next = false;

      // block i thread join until i thread progress is 1.
      // report progress in the meantime.
      while (!next)
      {
        next = tthreads[i]->progress >= 1;
        double progress = 0.0;

        // if i progresses reached 1, go to next block
        for (int p=0; p<it->len; p++)
        {
          progress += (tthreads[p]->progress / it->len);
        }
        
        if (indexer.on_progress != NULL)
        {
          report = progress / (double) (thread_it_count);
          indexer.on_progress(report + reported_progress, &indexer.payload);
        }
      }

      // join next thread.
      if (pthread_join(thread_ids[i], (void**) &result_chunk) != 0)
      {
        perror("can't join pthread");
        exit(1);
      }

      f_chunk* result = (f_chunk*) result_chunk;
      line_count += result->line_count;

      chunks[result->current] = result;
    }

    reported_progress += report;

    f_chunk* final_chunk;
    f_chunk_array_reverse_reduce(&final_chunk, 0, chunks, it->len);

    final_chunk->line_count = line_count;

    /* 
      by default, this indexer uses f_lookup_file.

      if we want to keep everything in memory, we could also use
      `f_lookup_mem`.
    */
    bool init_lookup = lookup == NULL ? true : false;
    bool last_lookup = itc == 0;

    f_lookup_file_from_chunk(&lookup, final_chunk, index_filename, init_lookup, last_lookup);

    /* free allocations */
    f_chunk_array_free(chunks, it->len);

    for(int z=0; z<it->len; z++)
    {
      free(tthreads[z]);
    }
    f_indexer_threads_free(it);

    free(tthreads);
    free(thread_ids);
  }

  fclose(fp);
  f_index* index;
  f_index_init(&index, indexer.filename, indexer.filename_len, lookup, NULL);

  return index;
}

#endif
