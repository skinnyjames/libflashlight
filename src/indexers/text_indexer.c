#ifndef FLASHLIGHT_INDEXERS_TEXT
#define FLASHLIGHT_INDEXERS_TEXT
#include <pthread.h>
#include <libdill.h>
#include <signal.h>
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
  const size_t buffer_size = ic->count;
  size_t total_bytes_offset = ic->from;

  uint8_t* buffer = malloc(sizeof(*buffer) * buffer_size);
  if (buffer == NULL)
  {
    f_log(F_LOG_ERROR, "failed to allocate read buffer");
    chsend(done, NULL, sizeof(f_chunk*), -1);
    return;
  }

  f_bytes_node* last_chunk_node = NULL;
  f_bytes_node* start_node = NULL;

  // ssize_t bytes_read;
  bool head = true;
  unsigned int line_count = 0u;

  const ssize_t bytes_read = pread(fd, buffer, buffer_size, total_bytes_offset);
  if (bytes_read == -1)
  {
    perror("failed to pread on file");
    chsend(done, NULL, sizeof(f_chunk*), -1);
    return;
  }

  int pos = 0;

  while (pos < bytes_read)
  {
    total_bytes_offset++;
    if ((char) buffer[pos] == '\n')
    {
      line_count++;

      f_bytes* bytes;
      if (f_bytes_new(&bytes, true, total_bytes_offset) == -1)
      {
        chsend(done, NULL, sizeof(f_chunk*), -1);
        return;
      }

      f_bytes_node* node;
      if (f_bytes_node_new(&node, bytes) == -1)
      {
        chsend(done, NULL, sizeof(f_chunk*), -1);
        return;
      }

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
  F_MTRIM(0);

  f_chunk* chunk;
  if (f_chunk_new(&chunk, ic->index, start_node, last_chunk_node) == -1)
  {
    chsend(done, NULL, sizeof(chunk), -1);
    return;
  }

  chunk->line_count = line_count;

  if (chsend(done, &chunk, sizeof(chunk), -1) != 0)
  {
    f_log(F_LOG_WARN, "couldn't send channel message to thread");
  }
}

void* f_index_text_chunk(void* payload)
{
  f_text_thread* tthread = (f_text_thread*) payload;

  unsigned int line_count = 0u;

  f_indexer_chunks* ic;
  if (f_indexer_chunks_init(&ic, tthread->concurrency, tthread->buffer_size, tthread->total_bytes_count, tthread->from, tthread->to) == -1)
  {
    f_log(F_LOG_ERROR, "cannot init indexer chunks");
    pthread_exit(NULL);
  }

  f_chunk** chunk_array;
  if (f_chunk_array_new(&chunk_array, ic->len) == -1)
  {
    f_log(F_LOG_ERROR, "cannot init chunk array");
    pthread_exit(NULL);
  }

  int chunks_finished = 0;
  int b = bundle();
  int chv[2];
  int rc = chmake(chv);
  if (rc == -1)
  {
    f_log(F_LOG_ERROR, "cannot create concurrency channel");
    pthread_exit(NULL);
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

      f_log(F_LOG_DEBUG, "[%d] [%lu] [cc: %d] [buf: %zu] chunk start %u [total: %zu]", tthread->thread, index, c, chunk->count, chunk->from, chunk->from + chunk->count);

      if (bundle_go(b, f_index_text_bytes(tthread->fd, send, chunk, tthread->thread)) == -1)
      {
        f_log(F_LOG_ERROR, "cannot run coroutine for chunk %d", index);
        pthread_exit(NULL);
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
        f_log(F_LOG_ERROR, "cannot receive chunk");
        pthread_exit(NULL);
      }

      if (chunk == NULL)
      {
        f_log(F_LOG_ERROR, "chunk is NULL");
        pthread_exit(NULL);
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
  if (f_chunk_array_reverse_reduce(&ret, tthread->thread, chunk_array, ic->len) == -1)
  {
    f_log(F_LOG_ERROR, "couldn't reduce chunk array");
    pthread_exit(NULL);
  }

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
  f_chunk* result_chunk;

  // get filehandle and total bytes. make nonblocking.
  FILE* fp = fopen(indexer.filename, "rb");
  if (fp == NULL)
  {
    f_log(F_LOG_ERROR, "Cannot open file for reading");
    return NULL;
  }

  int fd = fileno(fp);
  if (fd == -1)
  {
    return NULL;
  }

  int flags = fcntl(fd, F_GETFL, 0);
  if (flags == -1)
  {
    return NULL;

  }
  
  if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
  {
    f_log(F_LOG_ERROR, "Cannot set file flags");
    return NULL;
  }

  if (fseek(fp, 0, SEEK_END) == -1)
  {
    return NULL;
  }
  
  long int total_bytes_count = ftell(fp);
  if (total_bytes_count < 0)
  {
    f_log(F_LOG_ERROR, "Got a negative bytesize for file");
    return NULL;
  }

  if (fseek(fp, 0, SEEK_SET) == -1)
  {
    return NULL;
  }

  double reported_progress = 0.0;
  size_t max_bytes_per_iteration = indexer.max_bytes_per_iteration;
  int thread_it_count = (int) ceil((double) total_bytes_count / (double) (max_bytes_per_iteration));
  if (thread_it_count <= 0)
  {
    thread_it_count = 1;
  }

  f_log(F_LOG_DEBUG, "max bytes per iteration %zu, thread it count: %d", max_bytes_per_iteration, thread_it_count);

  f_lookup_file* lookup = NULL;
  size_t index_filename_len = 12 + strlen(indexer.lookup_dir);
  char random[10];
  char* index_filename = malloc(sizeof(char) * index_filename_len);
  rand_string(random, 10);
  cwk_path_join(indexer.lookup_dir, random, index_filename, index_filename_len);

  for (int itc=thread_it_count - 1; itc>=0; itc--)
  {
    unsigned long int thread_it_start = itc * max_bytes_per_iteration;
    size_t local_max_bytes_per_iteration = max_bytes_per_iteration;

    if (thread_it_start + max_bytes_per_iteration > total_bytes_count)
    {
      local_max_bytes_per_iteration = total_bytes_count - thread_it_start;
    }

    /* 
      lets try to iterate over x bytes at one time to keep consistent
      memory usage...
    */

    f_indexer_threads* it;
    if (f_indexer_threads_init(&it, indexer.threads, local_max_bytes_per_iteration, indexer.buffer_size, thread_it_start) == -1)
    {
      // TODO: free allocations
      f_log(F_LOG_ERROR, "Could not allocate indexer");
      return NULL;
    }

    // setup pthreads and thread chunks array.
    f_chunk** chunks;
    if (f_chunk_array_new(&chunks, it->len) == -1)
    {
      // TODO: free allocations
      f_log(F_LOG_ERROR, "Could not allocate chunks");
      return NULL;
    }

    f_text_thread** tthreads = malloc(sizeof(*tthreads) * it->len);
    if (tthreads == NULL)
    {
      // TODO: free allocations
      return NULL;
    }
    pthread_t* thread_ids = malloc(sizeof(pthread_t) * it->len);
    if (thread_ids == NULL)
    {
      // TODO: free allocations
      return NULL;
    }

    unsigned int line_count = 0u;

    // spawn threads.
    for (int i=0; i<it->len; i++)
    {
      // add extra container.
      f_text_thread* tthread = malloc(sizeof(*tthread));
      if (tthread == NULL)
      {
        // TODO: free allocations
        return NULL;
      }
      tthread->fd = fd;
      tthread->from = it->threads[i].from;
      tthread->to = it->threads[i].to;
      tthread->total_bytes_count = max_bytes_per_iteration;
      tthread->buffer_size = it->threads[i].buffer_size;
      tthread->concurrency = indexer.concurrency;
      tthread->thread = i;
      tthread->progress = (double) 0.0;

      tthreads[i] = tthread;
      if (pthread_create(&thread_ids[i], NULL, f_index_text_chunk, tthreads[i]) != 0)
      {
        // kill already created threads.
        for (int ti=0; ti<i; ti++)
        {
          pthread_kill(thread_ids[ti], SIGINT);
        }
        f_log(F_LOG_ERROR, "Couldn't create thread %d", i);
        return NULL;
      }
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
        perror("can't join thread - results may be incomplete.");
      }

      f_chunk* result = (f_chunk*) result_chunk;
      if (result == NULL)
      {
        f_log(F_LOG_ERROR, "result chunk %d is NULL", i);
        // kill the rest of the threads
        for (int ti=i; ti<it->len; ti++)
        {
          pthread_kill(thread_ids[ti], SIGINT);
        }
        return NULL;
      }

      line_count += result->line_count;

      chunks[result->current] = result;
    }

    reported_progress += report;

    f_chunk* final_chunk;
    if (f_chunk_array_reverse_reduce(&final_chunk, 0, chunks, it->len) == -1)
    {
      // todo free allocations
      f_log(F_LOG_ERROR, "failed to reduce chunk");
      return NULL;
    }

    final_chunk->line_count = line_count;

    /* 
      by default, this indexer uses f_lookup_file.

      if we want to keep everything in memory, we could also use
      `f_lookup_mem`.
    */
    bool init_lookup = lookup == NULL ? true : false;
    bool last_lookup = itc == 0;

    if (f_lookup_file_from_chunk(&lookup, final_chunk, index_filename, init_lookup, last_lookup) == -1)
    {
      f_log(F_LOG_ERROR, "failed to create index");
      return NULL;
    }

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

  if (fclose(fp) != 0)
  {
    f_log(F_LOG_WARN, "cannot close file descriptor");
  }

  f_index* index;
  if (f_index_init(&index, indexer.filename, indexer.filename_len, lookup, NULL) == -1)
  {
    f_log(F_LOG_ERROR, "failed to initialize index");
    return NULL;
  }

  return index;
}

#endif
